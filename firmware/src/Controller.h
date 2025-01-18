#pragma once
#include <FastLED.h>
#include "WiFi.h"
#include <esp_wifi.h>
#include "sACN.h"
#include <ArduinoJson.h>
#include <queue>
#include <stdint.h>

// ===== CONSTANT ========================================================================
#define BAUD_RATE 9600                      // debug connection
#define DMX_SIZE 512                        // dmx universe size (always 512)
#define DATA_PIN 5                          // hardware data pin
#define NUM_PXLS 3                          // rgb = 3 bytes
#define LED_TYPE WS2812B                    // ledstrip type
// =======================================================================================

// ----- Deployment params ---------------------------------------------------------------
#define STANDALONE                            // if on portable (5V) mode
#define ENABLE_LOGGING                        // enables logging
// ---------------------------------------------------------------------------------------

// ----- Wifi credentials ----------------------------------------------------------------
#define WIFI_SSID "Ledique"                 // wifi ssid (the same network as sacn master)
#define WIFI_PASS "dasenebipovezau"         // wifi password
// ---------------------------------------------------------------------------------------

// ----- Hardware LED params -------------------------------------------------------------
#define NUM_LEDS 64                        // number of hardware leds
#define LED_SIZE (NUM_LEDS * NUM_PXLS)      // total size in bytes
// ---------------------------------------------------------------------------------------

// ----- DMX params ----------------------------------------------------------------------
#define UNIVERSE 3                         // DMX universe
#define ADDR_OFFSET 0                       // address offset in said universe
#define NUM_GROUPS NUM_LEDS                      // this sets number of pixels
// ---------------------------------------------------------------------------------------

// ===== PARAMETER DEFINES ===============================================================
// setup standalone params
#ifdef STANDALONE
  #define WIFI_BRIGHTNESS 60
  #define WIFI_DELAY 50
#else
  #define WIFI_BRIGHTNESS 255
  #define WIFI_DELAY 10
#endif

// define logging functions
#ifdef ENABLE_LOGGING
  #define LOG(pattern) Serial.printf(pattern)
  #define LOGF(pattern, args...) Serial.printf(pattern, args)
#else
  #define LOG(pattern)
  #define LOGF(pattern, args...)
#endif

// we cant have more groups than leds
#if NUM_GROUPS > NUM_LEDS
  #define NUM_GROUPS NUM_LEDS
#endif
// =======================================================================================



class Controller {
  Controller() {}
  
  void setupWifi();
  void setupSacn();

  // transfer data from dmx to ledbuffer (group if necesarry)
  void update();
  
public:
  Controller(const Controller& other) = delete;

  // returns singleton instance
  static Controller& get() {
    static Controller instance;
    return instance;
  }

  // main init function; class can be reinitialised
  void init(uint8_t uni = UNIVERSE, uint16_t dmxAddressOffset = ADDR_OFFSET, uint16_t numberOfGroups = NUM_LEDS);
  
  // retrieve dmx data
  void updateLoop();

  // wifi connect annimation
  void playIdleAnimation();

  // functions for sending repot
  void clearDiffQueue(JsonArray& jarray);
  void sendUdpPacket(JsonDocument& doc);
  void sendReport();

  // ledstrip interactions
  inline void clear() { memset(ledBuffer, 0, LED_SIZE); FastLED.show(); }

  // threading functions
  void newPacket();
  void printNewRecv();
  void updateFramerate();
  void seqDiff();

private:
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
    
};