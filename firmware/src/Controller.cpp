#include "Controller.h"

void recv_dmxReceived()
{
  Controller::get().newPacket();
}

void recv_newSource()
{
  Controller::get().printNewRecv();
}

void recv_framerate()
{
  Controller::get().updateFramerate();
}

void recv_seqdiff()
{
  Controller::get().seqDiff();
}

void recv_timeOut()
{
  // Serial.println("Timeout!");
}

void Controller::init(uint8_t uni, uint16_t dmxAddressOffset, uint16_t numberOfGroups) {
  universe = uni;
  dmxAddrOffset = dmxAddressOffset;
  numGroups = numberOfGroups;

  static bool inited = false;

  if(!inited) {
    Serial.begin(BAUD_RATE);

    setupWifi();
    setupSacn();

    mutex = xSemaphoreCreateMutex();
    cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>((CRGB *)ledBuffer, NUM_LEDS);

    inited = true;
  }
  else {
    delete recv;
    setupSacn();
  }
}

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
  recv->callbackDMX(recv_dmxReceived);
  recv->callbackSource(recv_newSource);
  recv->callbackFramerate(recv_framerate);
  recv->callbackSeqDiff(recv_seqdiff);
  recv->callbackTimeout(recv_timeOut);
  recv->begin(universe);
}

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

void Controller::updateLoop() {
  recv->update();
  FastLED.show();
}

// void Controller::playIdleAnimation() {
//   ledBuffer[((millis() / 10) % (NUM_LEDS * 3))] = 255;
//   ledBuffer[((millis() / 10) % (NUM_LEDS * 3)) - 1] = 0;  
// }

// void Controller::checkNetwork() { 
//   if (WiFi.status() != WL_CONNECTED)
//   {
//     TaskHandle_t animation = NULL;
//     LOG("Lost connection\n");
//     xTaskCreate(
//         playIdleAnimation, // Task function
//         "Animation",       // Name of the task (for debugging)
//         5000,              // Stack size in words
//         NULL,              // Parameter passed to the task
//         2,                 // Task priority
//         &animation         // Handle to the task
//     );
//     while (WiFi.status() != WL_CONNECTED)
//     {
//       vTaskDelay(100);
//     }
    
//     LOG("Connected\n");
//     vTaskDelete(animation);
//   }
// }

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

void Controller::sendUdpPacket(JsonArray& doc) {
  udp.beginPacket(WiFi.broadcastIP(), 12345);
  serializeJson(doc, udp);
  // udp.printf("heap %d, cycle: %d, chip cores: %d, PSram: %d, CPU Freq %d, heapsize: %d, maxHeap: %d, maxPSram: %d", ESP.getFreeHeap(), ESP.getCycleCount(), ESP.getChipCores(), ESP.getFreePsram(), ESP.getCpuFreqMHz(), ESP.getHeapSize(), ESP.getMinFreeHeap(), ESP.getMinFreePsram());
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

  sendUdpPacket(jsonArray);
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