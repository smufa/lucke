#include <FastLED.h>
#include "WiFi.h"
#include <esp_wifi.h>
#include "sACN.h"
#include <ArduinoJson.h>
#include <queue>
#include <stdint.h>


#define ENABLE_LOGGING
#ifdef ENABLE_LOGGING
  #define LOG(pattern) Serial.printf(pattern)
  #define LOGF(pattern, args...) Serial.printf(pattern, args)
#else
  #define LOG(pattern)
  #define LOGF(pattern, args...)
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


SemaphoreHandle_t mutex;


void recv_dmxReceived();
void recv_newSource();
void recv_framerate();
void recv_seqdiff();
void recv_timeOut();


class Controller {
  Controller() {}
  
public:
  Controller(const Controller& other) = delete;

  uint8_t universe = 5;
  uint16_t dmxAddrOffset = 0;
  uint16_t numGroups = 100;  
  uint8_t ledBuffer[LED_SIZE] = {};
  uint8_t dmxBuffer[DMX_SIZE] = {};  

  int droppedPackets = 0;
  int lastDMXFramerate = 0;
  std::queue<uint8_t> packetDiff;

  CLEDController *cled;

  WiFiUDP udp;
  Receiver* recv;

  static Controller& get(){
    static Controller s_Instance;
    return s_Instance;
  }

  void init() {
      setupWifi();
      setupSacn();

      cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>((CRGB *)ledBuffer, NUM_LEDS);
  }

  void setupWifi() {
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

  void setupSacn() {
    // setup sacn
    recv = new Receiver(udp);
    recv->callbackDMX(recv_dmxReceived);
    recv->callbackSource(recv_newSource);
    recv->callbackFramerate(recv_framerate);
    recv->callbackSeqDiff(recv_seqdiff);
    recv->callbackTimeout(recv_timeOut);
    recv->begin(universe);
  }

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

  void clearDiffQueue(JsonArray& jarray) {
    while (!packetDiff.empty())
    {
      jarray.add(packetDiff.front());
      packetDiff.pop();
    }
  }

  void sendUdpPacket(JsonArray& doc) {
    udp.beginPacket(WiFi.broadcastIP(), 12345);
    serializeJson(doc, udp);
    // udp.printf("heap %d, cycle: %d, chip cores: %d, PSram: %d, CPU Freq %d, heapsize: %d, maxHeap: %d, maxPSram: %d", ESP.getFreeHeap(), ESP.getCycleCount(), ESP.getChipCores(), ESP.getFreePsram(), ESP.getCpuFreqMHz(), ESP.getHeapSize(), ESP.getMinFreeHeap(), ESP.getMinFreePsram());
    udp.endPacket();
  }


  void newPacket() {
    recv->dmx(dmxBuffer);
    update();
  }

  void printNewRecv() {
    LOGF("%s\n", recv->name());
  }

  void updateFramerate() {
    lastDMXFramerate = recv->framerate();
  }

  void seqDiff() {
    uint8_t diff = recv->seqdiff();
    if (xSemaphoreTake(mutex, 0) == pdTRUE)
    {
      packetDiff.push(diff);
      xSemaphoreGive(mutex);
    }
  }

};

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);    // Set your gateway
// IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask

// Misc

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


void statReportLoop(void *)
{
  while (true)
  {
    JsonDocument doc;
    doc["universe"] = Controller::get().universe;
    doc["heap_size"] = ESP.getHeapSize();
    doc["heap_free"] = ESP.getFreeHeap();
    doc["local_ip"] = WiFi.localIP();
    doc["WIFI_SSID"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["last_DMX_framerate"] = Controller::get().lastDMXFramerate;

    // add packet sequence diff to json
    doc["seq_diff"] = JsonDocument();
    JsonArray diffArray = doc["seq_diff"].to<JsonArray>();
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
    {
      Controller::get().clearDiffQueue(diffArray);
      xSemaphoreGive(mutex);
    }

    // add first 5 leds
    doc["first_5_leds"] = JsonDocument();
    JsonArray jsonArray = doc["first_5_leds"].to<JsonArray>();
    uint8_t* ledBuffer = Controller::get().getLEDBuffer();
    for (int i = 0; i < 15; i++)
    {
      jsonArray.add(ledBuffer[i]);
    }

    Controller::get().sendUdpPacket(jsonArray);
    vTaskDelay(1000);
  }
}

void playIdleAnimation(void *)
{
  while (true)
  {
    uint8_t* ledBuffer = Controller::get().getLEDBuffer();
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

void dmxLoop(void *)
  {
    while (true)
    {
      Controller::get().recv->update();
      FastLED.show();
      vTaskDelay(15);
    }
  }

void setup()
{
  Serial.begin(BAUD_RATE);
  mutex = xSemaphoreCreateMutex();
  
  // setup wifi
  Controller::get().init();
  

  // create all tasks
  xTaskCreate(dmxLoop, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(checkNetwork, "Wifi check", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(statReportLoop, "Logging", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

void loop()
{
  vTaskDelete(NULL);
}
