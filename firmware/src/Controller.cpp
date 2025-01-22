#include "Controller.h"

#if DIMENSION == DIMENSION_1D
void Controller::init(uint16_t numberOfGroups, uint8_t uni, uint16_t dmxAddressOffset) {
  universe = uni;
  dmxAddrOffset = dmxAddressOffset;
  numGroups = numberOfGroups;
  static bool inited = false;

  if(!inited) {
#ifdef ENABLE_LOGGING
    Serial.begin(BAUD_RATE);
#endif
    setupWifi();
    setupSacn();

    mutex = xSemaphoreCreateMutex();
    cled = &FastLED.addLeds<LED_TYPE, DATA_PIN, LED_ORDER>((CRGB *)ledBuffer, NUM_LEDS);

    inited = true;
  }
  else {
    delete recv;
    setupSacn();
  }
}
#else
void Controller::init2D(int wsize, int hsize, int width, int height, uint8_t uni, uint16_t dmxAddressOffset) {
  universe = uni;
  dmxAddrOffset = dmxAddressOffset;
  static bool inited = false;

  if(!inited) {
#ifdef ENABLE_LOGGING
    Serial.begin(BAUD_RATE);
#endif
    setupWifi();
    setupSacn();

    mutex = xSemaphoreCreateMutex();
    cled = &FastLED.addLeds<LED_TYPE, DATA_PIN, LED_ORDER>((CRGB *)ledBuffer, NUM_LEDS);

    inited = true;
  }
  else {
    delete recv;
    grid = Grid(wsize, hsize, width, height);
    setupSacn();
  }
}
#endif
// void Controller::init(int wsize, int hsize, int width, int height, uint8_t uni, uint16_t dmxAddressOffset) 
// {
//   universe = uni;
//   dmxAddrOffset = dmxAddressOffset;
//   static bool inited = false;

//   if(!inited) {
// #ifdef ENABLE_LOGGING
//     Serial.begin(BAUD_RATE);
// #endif
//     setupWifi();
//     setupSacn();
//     grid = Grid(wsize, hsize, width, height);

//     mutex = xSemaphoreCreateMutex();
//     cled = &FastLED.addLeds<LED_TYPE, DATA_PIN, LED_ORDER>((CRGB *)ledBuffer, NUM_LEDS);

//     inited = true;
//   }
//   else {
//     delete recv;
//     setupSacn();
//   }
// }


void Controller::setupWifi() {
  // setup mac address
  uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, universe}; // MAC Adress of your device
  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
  if (err == ESP_OK)
  {
      LOG("Success changing Mac Address\nS");
  }

  WiFi.setScanMethod(WIFI_FAST_SCAN);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void Controller::setupSacn() {
  // setup sacn
  recv = new Receiver(udp);
  recv->callbackDMX([](){ Controller::get().newPacket(); });
  recv->callbackSource([](){ Controller::get().printNewRecv(); });
  recv->callbackFramerate([](){ Controller::get().updateFramerate(); });
  recv->callbackSeqDiff([](){ Controller::get().seqDiff(); });
  recv->callbackTimeout([](){});
  recv->begin(universe);
}

#if DIMENSION == DIMENSION_1D

void Controller::update() {
  uint16_t groupSize = NUM_LEDS / numGroups;
  uint16_t ledIndex = 0;

  for (uint16_t i = 0; i < numGroups; i++) {
    for (uint16_t j = 0; j < groupSize; j++) {
      for (uint16_t k = 0; k < NUM_PXLS; k++) {
        // check if in bounds
        uint16_t dmxBufferIndex = dmxAddrOffset + i * NUM_PXLS + k;
        ledBuffer[ledIndex] = dmxBuffer[dmxBufferIndex];
        ledIndex++;
      }
    }
  }
}

#else

void Controller::update(){
  for(uint16_t y = 0; y < grid.nh; y++) {
    for(uint16_t x = 0; x < grid.nw; x++) {
      const auto& indexes = grid.getGridIndexes(x,y);
      int dmxIndex = dmxAddrOffset + (x * NUM_PXLS) + (y * NUM_PXLS) * grid.nw;
      // printf("dmxIndex = %d\n", dmxIndex);
      for(auto index : indexes) {
        for (uint16_t k = 0; k < NUM_PXLS; k++) {
          // LOGF("led[%d] = dmx [%d]\n", (index * NUM_PXLS + k), (dmxIndex + k));
          ledBuffer[index * NUM_PXLS + k] = dmxBuffer[dmxIndex + k]; 
        }
      }
    }
  }
}

#endif

void Controller::updateLoop() {
  recv->update();
  FastLED.show();
}

void Controller::playIdleAnimation() {
  ledBuffer[((millis() / 10) % LED_SIZE)] = WIFI_BRIGHTNESS;
  ledBuffer[(((millis() / 10) % LED_SIZE) - 1) % LED_SIZE] = 0;  
}

void Controller::clearDiffQueue(JsonArray& jarray) {
  if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
  {
    while (!packetDiff.empty())
    {
      jarray.add(packetDiff.front());
      packetDiff.pop();
    }
    xSemaphoreGive(mutex);
  }
}

void Controller::sendUdpPacket(JsonDocument& doc) {
  udp.beginPacket(WiFi.broadcastIP(), 12345);
  serializeJson(doc, udp);
  udp.endPacket();
}

void Controller::sendReport() {
  JsonDocument doc;
  doc["universe"] = universe;
  doc["heap_size"] = ESP.getHeapSize();
  doc["heap_free"] = ESP.getFreeHeap();
  doc["local_ip"] = WiFi.localIP();
  doc["WIFI_SSID"] = WiFi.SSID();
  doc["rssi"] = WiFi.RSSI();
  doc["last_DMX_framerate"] = lastDMXFramerate;

  // add packet sequence diff to json
  doc["seq_diff"] = JsonDocument();
  JsonArray diffArray = doc["seq_diff"].to<JsonArray>();
  clearDiffQueue(diffArray);

  // add first 5 leds
  doc["first_5_leds"] = JsonDocument();
  JsonArray jsonArray = doc["first_5_leds"].to<JsonArray>();
  for (int i = 0; i < 15; i++)
  {
      jsonArray.add(ledBuffer[i]);
  }

  sendUdpPacket(doc);
}


void Controller::newPacket() {
  recv->dmx(dmxBuffer);
  update();
}

void Controller::printNewRecv() {
  LOGF("%s\n", recv->name());
}

void Controller::updateFramerate() {
  lastDMXFramerate = recv->framerate();
}

void Controller::seqDiff() {
  uint8_t diff = recv->seqdiff();
  if (xSemaphoreTake(mutex, 0) == pdTRUE)
  {
      packetDiff.push(diff);
      xSemaphoreGive(mutex);
  }
}