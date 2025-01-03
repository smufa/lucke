#include <FastLED.h>
#include "WiFi.h"
#include <esp_wifi.h>
#include "sACN.h"
#include <ArduinoJson.h>
#include <queue>
#include <stdint.h>


#define ENABLE_LOGGING
#ifdef ENABLE_LOGGING
  #define LOG(pattern, args...) Serial.printf(pattern, args)
#else
  #define LOG(pattern, args...)
#endif

// #define BAUD_RATE 115200
#define BAUD_RATE 9600

// Wifi credentials
#define WIFI_SSID "Ledique"
#define WIFI_PASS "dasenebipovezau"

// DMX packet size
#define DMX_SIZE 512
// data pin for leds
#define DATA_PIN 5

#define NUM_LEDS 100
#define NUM_PXLS 3
#define LED_SIZE (NUM_LEDS * NUM_PXLS)



struct Controller {
  uint8_t universe = 5;
  uint16_t dmxAddrOffset = 0;
  uint16_t numGroups = 100;  
  uint8_t ledBuffer[LED_SIZE] = {};
  uint8_t dmxBuffer[DMX_SIZE] = {};  

  int droppedPackets = 0;
  int lastDMXFramerate = 0;
  std::queue<uint8_t> packetDiff;

  inline uint8_t* getLEDBuffer () { return ledBuffer; }
  inline uint8_t* getDMXBuffer () { return dmxBuffer; }

  void update() {
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

  void pushToQueue(uint8_t diff){
    packetDiff.push(diff);
  }

  void clearDiffQueue(JsonArray& jarray) {
    while (!packetDiff.empty())
    {
      jarray.add(packetDiff.front());
      packetDiff.pop();
    }
  }

  void setupWifi(){
    WiFi.setScanMethod(WIFI_FAST_SCAN);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }

  // void taskFramerate(){

  // }

  // void taskDMXReceived(){
    
  // }

  // void taskTimeout(){
    
  // }
  // void taskNewSource(){
    
  // }
};


CLEDController *cled;
Controller controller;

// Network shit
uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0xDD}; // MAC Adress of your device
WiFiUDP udp;
Receiver recv(udp);

SemaphoreHandle_t mutex;

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);    // Set your gateway
// IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask

// Misc

void dmxReceived()
{
  recv.dmx(controller.getDMXBuffer());
  controller.update();
}

void newSource()
{
  LOG("%s\n", recv.name());
}

void framerate()
{
  controller.lastDMXFramerate = recv.framerate();
}

void seqdiff()
{
  uint8_t diff = recv.seqdiff();
  if (xSemaphoreTake(mutex, 0) == pdTRUE)
  {
    controller.pushToQueue(diff);
    xSemaphoreGive(mutex);
  }
}

void timeOut()
{
  // Serial.println("Timeout!");
}

void dmxLoop(void *)
{
  while (true)
  {
    recv.update();
    FastLED.show();
    vTaskDelay(15);
  }
}

void statReportLoop(void *)
{
  while (true)
  {
    JsonDocument doc;
    doc["universe"] = controller.universe;
    doc["heap_size"] = ESP.getHeapSize();
    doc["heap_free"] = ESP.getFreeHeap();
    doc["local_ip"] = WiFi.localIP();
    doc["WIFI_SSID"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["last_DMX_framerate"] = controller.lastDMXFramerate;

    // add packet sequence diff to json
    doc["seq_diff"] = JsonDocument();
    JsonArray diffArray = doc["seq_diff"].to<JsonArray>();
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
    {
      controller.clearDiffQueue(diffArray);
      xSemaphoreGive(mutex);
    }

    // add first 5 leds
    doc["first_5_leds"] = JsonDocument();
    JsonArray jsonArray = doc["first_5_leds"].to<JsonArray>();
    uint8_t* ledBuffer = controller.getLEDBuffer();
    for (int i = 0; i < 15; i++)
    {
      jsonArray.add(ledBuffer[i]);
    }

    udp.beginPacket(WiFi.broadcastIP(), 12345);

    serializeJson(doc, udp);
    // udp.printf("heap %d, cycle: %d, chip cores: %d, PSram: %d, CPU Freq %d, heapsize: %d, maxHeap: %d, maxPSram: %d", ESP.getFreeHeap(), ESP.getCycleCount(), ESP.getChipCores(), ESP.getFreePsram(), ESP.getCpuFreqMHz(), ESP.getHeapSize(), ESP.getMinFreeHeap(), ESP.getMinFreePsram());
    udp.endPacket();
    vTaskDelay(1000);
  }
}

void playIdleAnimation(void *)
{
  while (true)
  {
    uint8_t* ledBuffer = controller.getLEDBuffer();
    ledBuffer[((millis() / 10) % (NUM_LEDS * 3))] = 255;
    ledBuffer[((millis() / 10) % (NUM_LEDS * 3)) - 1] = 0;
    vTaskDelay(5);
  }
}

void checkNetwork(void *)
{
  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      TaskHandle_t animation = NULL;
      LOG("Lost connection\n");
      xTaskCreate(
          playIdleAnimation, // Task function
          "Animation",       // Name of the task (for debugging)
          5000,              // Stack size in words
          NULL,              // Parameter passed to the task
          2,                 // Task priority
          &animation         // Handle to the task
      );
      while (WiFi.status() != WL_CONNECTED)
      {
        // WiFi.begin(WIFI_SSID, WIFI_PASS);
        vTaskDelay(100);
      }
      LOG("Connected\n");
      vTaskDelete(animation);
    }
    vTaskDelay(100);
  }
}

void setup()
{
  Serial.begin(BAUD_RATE);
  mutex = xSemaphoreCreateMutex();
  
  // set mac address
  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
  if (err == ESP_OK)
  {
    LOG("Success changing Mac Address\n");
  }

  // setup wifi
  controller.setupWifi();

  // setup sacn
  recv.callbackDMX(dmxReceived);
  recv.callbackSource(newSource);
  recv.callbackFramerate(framerate);
  recv.callbackSeqDiff(seqdiff);
  recv.callbackTimeout(timeOut);
  recv.begin(controller.universe);

  // init fastled
  cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>((CRGB *)controller.getLEDBuffer(), NUM_LEDS);

  // create all tasks
  xTaskCreate(dmxLoop, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(checkNetwork, "Wifi check", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(statReportLoop, "Logging", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

void loop()
{
  vTaskDelete(NULL);
}
