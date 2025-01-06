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


void recv_dmxReceived();
void recv_newSource();
void recv_framerate();
void recv_seqdiff();
void recv_timeOut();


class Controller {
  Controller() {}
  
public:
  Controller(const Controller& other) = delete;

  uint8_t universe = 1;
  uint16_t dmxAddrOffset = 0;
  uint16_t numGroups = 100;  
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

  void init(uint8_t uni, uint16_t dmxAddressOffset = 0, uint16_t numberOfGroups = NUM_LEDS);
  void setupWifi();
  void setupSacn();
  
  inline uint8_t* getLEDBuffer () { return ledBuffer; }
  inline uint8_t* getDMXBuffer () { return dmxBuffer; }

  void update();
  void updateLoop();
//   void playIdleAnimation();
//   void checkNetwork();

  void clearDiffQueue(JsonArray& jarray);
  void sendUdpPacket(JsonArray& doc);
  void sendReport();

  void newPacket();
  void printNewRecv();
  void updateFramerate();
  void seqDiff();

private:
    
};