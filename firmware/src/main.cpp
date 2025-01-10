#include "Controller.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);    // Set your gateway
// IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask


void statReportLoop(void *)
{
  while (true)
  {
    Controller::get().sendReport();
    vTaskDelay(1000);
  }
}

void playIdleAnimation(void *)
{
  while (true)
  {
#define BRIGHTNESS 255
#define WIFIANI_SLEEP 5
#ifdef STANDALONE
  #define BRIGHTNESS 40
  #define WIFIANI_SLEEP 50
#endif
    uint8_t* ledBuffer = Controller::get().getLEDBuffer();
    ledBuffer[((millis() / 50) % LED_SIZE)] = BRIGHTNESS;
    ledBuffer[(((millis() / 50) -1) % LED_SIZE)] = 0;
    vTaskDelay(WIFIANI_SLEEP);
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
      Controller::get().updateLoop();
      vTaskDelay(15);
    }
  }

void setup()
{  
  // setup wifi, sacn, and other stuff
  Controller::get().init(UNIVERSE, ADDR_OFFSET, NUM_GROUPS);

  // create all tasks
  xTaskCreate(dmxLoop, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(checkNetwork, "Wifi check", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(statReportLoop, "Logging", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

void loop()
{
  vTaskDelete(NULL);
}
