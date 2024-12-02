#include <FastLED.h>
#include <Arduino.h>

// LED shit
#define NUM_LEDS 101
#define DATA_PIN 5
CLEDController *cled;
// CRGB leds[NUM_LEDS];
uint8_t cbuffer[512];
int t;

void all_on(uint8_t *cbuffer) {
  for (int i = 0; i < NUM_LEDS * 3; i++) {
    cbuffer[i] = 255;
  }
}

void all_off(uint8_t *cbuffer) {
  for (int i = 0; i < NUM_LEDS * 3; i++) {
    cbuffer[i] = 0;
  }
}

void setup()
{
  Serial.begin(9600);
  delay(100);

  cled = &FastLED.addLeds<WS2815, DATA_PIN, RGB>((CRGB*)cbuffer, NUM_LEDS);
  t = 0;

  for (int i = 0; i < 3; i++) {
    all_on(cbuffer);
    FastLED.show();
    delay(1000);
    all_off(cbuffer);
    FastLED.show();
    delay(1000);
  }
}

void loop()
{
  if (t % 4 == 0) 
  {
    cbuffer[0] = 255;
    cbuffer[4] = 0;
    cbuffer[3 * 99 + 2] = 0;
    cbuffer[3 * 100] = 0;
  } 
  else if (t % 4 == 1)
  {
    cbuffer[0] = 0;
    cbuffer[4] = 255;
    cbuffer[3 * 99 + 2] = 0;
    cbuffer[3 * 100] = 0;
  } 
  else if (t % 4 == 2)
  {
    cbuffer[0] = 0;
    cbuffer[4] = 0;
    cbuffer[3 * 99 + 2] = 255;
    cbuffer[3 * 100] = 0;
  } else 
  {
    cbuffer[0] = 0;
    cbuffer[4] = 0;
    cbuffer[3 * 99 + 2] = 0;
    cbuffer[3 * 100] = 255;
  }
  t++;
  FastLED.show();
  delay(200);
}