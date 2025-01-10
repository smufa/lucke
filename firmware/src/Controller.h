#pragma once
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

// basically sets max brightness on wifi search animation if defined
#define STANDALONE

// Hardware LED params
#define NUM_PXLS 3                        // rgb = 3 bytes
#define NUM_LEDS 10                       // number of hardware leds
#define LED_SIZE (NUM_LEDS * NUM_PXLS)    // total size in bytes

// DMX params
#define UNIVERSE 5                        // DMX universe
#define ADDR_OFFSET 0                     // address offset in said universe
#define NUM_GROUPS 10                     // this sets number of pixels

// we cant have more groups than leds
#if NUM_GROUPS > NUM_LEDS
  #define NUM_GROUPS NUM_LEDS
#endif

// CONSTANT
#define BAUD_RATE 9600                    // debug connection
#define DMX_SIZE 512                      // dmx universe size (always 512)
#define DATA_PIN 5                        // hardware data pin

// Wifi credentials
#define WIFI_SSID "Ledique"               // wifi ssid (the same network as sacn master)
#define WIFI_PASS "dasenebipovezau"       // wifi password


void recv_dmxReceived();
void recv_newSource();
void recv_framerate();
void recv_seqdiff();
void recv_timeOut();


class Controller {
  Controller() {}
  
public:
  Controller(const Controller& other) = delete;

  uint8_t universe = UNIVERSE;
  uint16_t dmxAddrOffset = ADDR_OFFSET;
  uint16_t numGroups = NUM_GROUPS;  
  uint8_t ledBuffer[LED_SIZE] = {};
  uint8_t dmxBuffer[DMX_SIZE] = {};  

  int droppedPackets = 0;
  int lastDMXFramerate = 0;
  std::queue<uint8_t> packetDiff;

  CLEDController *cled;
  SemaphoreHandle_t mutex;

  WiFiUDP udp;
  Receiver* recv;

  static Controller& get() {
    static Controller instance;
    return instance;
  }

  void init(uint8_t uni = UNIVERSE, uint16_t dmxAddressOffset = ADDR_OFFSET, uint16_t numberOfGroups = NUM_LEDS);
  void setupWifi();
  void setupSacn();
  
  inline uint8_t* getLEDBuffer () { return ledBuffer; }
  inline uint8_t* getDMXBuffer () { return dmxBuffer; }

  void update();
  void updateLoop();
//   void playIdleAnimation();
//   void checkNetwork();

  void clearDiffQueue(JsonArray& jarray);
  void sendUdpPacket(JsonDocument& doc);
  void sendReport();

  void newPacket();
  void printNewRecv();
  void updateFramerate();
  void seqDiff();

private:
    
};