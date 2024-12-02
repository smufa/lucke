#include <FastLED.h>
#include "WiFi.h"
#include "sACN.h"

// LED shit
#define NUM_LEDS 100
#define DATA_PIN 5
CLEDController *cled;
CRGB leds[NUM_LEDS];
uint8_t cbuffer[512];

// Network shit
uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0x48}; // MAC Adress of your device
WiFiUDP udp;
Receiver recv(udp); // universe 1
const char *ssid = "nLa";
const char *password = "tugicamalo";
// const char *ssid = "Smart Toilet";
// const char *password = "bbbbbbbbb";
// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);    // Set your gateway
// IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask

void dmxReceived()
{
  recv.dmx(cbuffer);

  // cled->setLeds((CRGB*)cbuffer, NUM_LEDS);
  // for (int i = 0; i < 100; i++) {
  //   leds[i] = CRGB(buffer[3 * i], buffer[3 * i + 1], buffer[3 * i + 2]);
  // }
  // leds[0] = CRGB(buffer[0], buffer[1], buffer[2]);
  // Serial.print("Led1:");
  // Serial.print(buffer[0]);
  // Serial.print(buffer[1]);
  // Serial.println(buffer[2]);
}

void newSource()
{
  // Serial.print("new soure name: ");
  Serial.println(recv.name());
}

void framerate()
{
  Serial.print("Framerate fps: ");
  Serial.println(recv.framerate());
}

void timeOut()
{
  // Serial.println("Timeout!");
}
// bool odd;

void dmxLoop(void *)
{
  for (;;)
  {
    recv.update();
    delay(1);
  }
}

void ledLoop(void *)
{
  for (;;)
  {
    FastLED.show();
    delay(1);
  }
}

void statReportLoop(void *)
{
  while (true)
  {
    // once we know where we got the inital packet from, send data back to that IP address and port
    udp.beginPacket("192.168.157.255", 12345);
    // Just test touch pin - Touch0 is T0 which is on GPIO 4.
    udp.printf("getHeapSize: %d, getFreeHeap: %d", ESP.getHeapSize(), ESP.getFreeHeap());
    // udp.printf("heap %d, cycle: %d, chip cores: %d, PSram: %d, CPU Freq %d, heapsize: %d, maxHeap: %d, maxPSram: %d", ESP.getFreeHeap(), ESP.getCycleCount(), ESP.getChipCores(), ESP.getFreePsram(), ESP.getCpuFreqMHz(), ESP.getHeapSize(), ESP.getMinFreeHeap(), ESP.getMinFreePsram());
    udp.endPacket();
    vTaskDelay(1000);
  }
}

void setup()
{
  setCpuFrequencyMhz(240);
  Serial.begin(9600);
  // WiFi.config(local_IP, gateway, subnet);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);         // Wait for 500ms before checking again
    Serial.print("."); // Print a dot to show progress
  }

  // esp_bluedroid_disable();
  // esp_bluedroid_deinit();
  // esp_bt_controller_disable();
  // esp_bt_controller_deinit();
  recv.callbackDMX(dmxReceived);
  recv.callbackSource(newSource);
  recv.callbackFramerate(framerate);
  recv.callbackTimeout(timeOut);
  recv.begin(1, true);
  Serial.println("sACN start");
  Serial.println(WiFi.localIP());
  Serial.println(portNUM_PROCESSORS);
  cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>((CRGB *)cbuffer, NUM_LEDS);
  // cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>(leds, NUM_LEDS);
  // randomSeed(analogRead(0));
  // odd = true;
  xTaskCreatePinnedToCore(dmxLoop, "DMX", 10000, NULL, 2 | portPRIVILEGE_BIT, NULL, 0);
  xTaskCreatePinnedToCore(ledLoop, "LED", 10000, NULL, 2 | portPRIVILEGE_BIT, NULL, 0);
  xTaskCreatePinnedToCore(statReportLoop, "status reporting", 10000, NULL, 2 | portPRIVILEGE_BIT, NULL, 0);
}

void loop()
{
  // recv.update();
  // FastLED.show();
  // Serial.println(WiFi.localIP());
  // if (WiFi.isConnected()) {
  // Serial.println("connected");
  // Serial.println(WiFi.localIP());
  // }
  // recv.update();
  // for (int i = 0; i < 100; i++) {
  //   if (odd) {
  //     leds[i] = CRGB::Black;
  //   } else {
  //     leds[i] = CRGB(random(0, 156), random(0, 156), random(0, 156));
  //   }
  // }
  // FastLED.show();
  // delay(50);
  // odd = !odd;
  vTaskDelete(NULL);
}
