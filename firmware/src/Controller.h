#pragma once
#include <FastLED.h>
#include "WiFi.h"
#include <esp_wifi.h>
#include "sACN.h"
#include <ArduinoJson.h>
#include <queue>
#include <vector>
#include <unordered_map>
#include <stdint.h>

// ===== CONSTANT ========================================================================
#define BAUD_RATE 9600                      // debug connection
#define DMX_SIZE 512                        // dmx universe size (always 512)
#define DATA_PIN 5                          // hardware data pin
#define NUM_PXLS 3                          // rgb = 3 bytes
#define LED_TYPE WS2812B                    // ledstrip type (FASTLED)
#define LED_ORDER GRB                       // led order type (FASTLED)

#define DIMENSION_1D 1                      // led group 1d
#define DIMENSION_2D 2                      // led group 2d
// =======================================================================================

// ----- Deployment params ---------------------------------------------------------------
#define STANDALONE                          // if on portable (5V) mode
#define ENABLE_LOGGING                      // enables logging
#define DIMENSION DIMENSION_2D
// ---------------------------------------------------------------------------------------

// ----- Wifi credentials ----------------------------------------------------------------
#define WIFI_SSID "Ledique"                 // wifi ssid (the same network as sacn master)
#define WIFI_PASS "dasenebipovezau"         // wifi password
// ---------------------------------------------------------------------------------------

// ----- Hardware LED params -------------------------------------------------------------
#define NUM_LEDS 64                         // number of hardware leds
#define LED_SIZE (NUM_LEDS * NUM_PXLS)      // total size in bytes
// ---------------------------------------------------------------------------------------

// ----- DMX params ----------------------------------------------------------------------
#define UNIVERSE 5                          // DMX universe
#define ADDR_OFFSET 0                       // address offset in said universe

#define NUM_GROUPS NUM_LEDS               // this sets number of pixels
#if DIMENSION == DIMENSION_2D
  #define GRID_WSIZE 2
  #define GRID_HSIZE 2
  #define GRID_WIDTH 8
  #define GRID_HEIGHT 8
#endif
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


#if DIMENSION == DIMENSION_2D

struct Grid {
  int width = GRID_WIDTH;
  int heigth = GRID_HEIGHT;
  int wsize = GRID_WSIZE;
  int hsize = GRID_HSIZE;
  int nw, nh;

  std::unordered_map<int, std::vector<int>> hash;

  Grid() : nw(width / wsize), nh(heigth / hsize) {};
  Grid(int wsize, int hsize, int width, int height) : wsize(wsize), hsize(hsize), nh(heigth / hsize) {};

  const std::vector<int> &getGridIndexes(int x, int y) {
    // TODO: Find bigger prime numbers or better hashing
    int hnum = x * 7741 + y * 7757;
    
    if(hash.find(hnum) != hash.end()) {
      return hash[hnum];
    }

    for(int yi = y * hsize; yi < (y + 1) * hsize; yi++) {
      for(int xi = x * wsize; xi < (x + 1) * wsize; xi++) {
        hash[hnum].push_back(xi + yi * width);
      }
    }

    return hash[hnum];
  }
};

#endif


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
#if DIMENSION == DIMENSION_1D
  void init(uint16_t numberOfGroups = NUM_LEDS, uint8_t uni = UNIVERSE, uint16_t dmxAddressOffset = ADDR_OFFSET);
#else
  void init(
            int wsize = GRID_WSIZE, 
            int hsize = GRID_HSIZE, 
            int width = GRID_WIDTH, 
            int height = GRID_HEIGHT,
            uint8_t uni = UNIVERSE, 
            uint16_t dmxAddressOffset = ADDR_OFFSET); 
#endif
  
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

#if DIMENSION == DIMENSION_1D
  uint16_t numGroups = NUM_GROUPS;
#endif

  uint8_t ledBuffer[LED_SIZE] = {};
  uint8_t dmxBuffer[DMX_SIZE] = {};  

  int droppedPackets = 0;
  int lastDMXFramerate = 0;
  std::queue<uint8_t> packetDiff;

  CLEDController *cled;
  SemaphoreHandle_t mutex;

  WiFiUDP udp;
  Receiver* recv;
  
#if DIMENSION == DIMENSION_2D
  Grid grid;
#endif
};