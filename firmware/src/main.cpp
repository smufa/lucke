#include "Controller.h"

// IPAddress local_IP(192, 168, 0, 150); // Set the desired IP address
// IPAddress gateway(192, 168, 0, 1);    // Set your gateway
// IPAddress subnet(255, 255, 255, 0);   // Set your subnet mask

// thread that sends reports via udp
void statReportLoop(void *)
{
  while (true)
  {
    Controller::get().sendReport();
    vTaskDelay(1000);
  }
}

// thread that plays idle animation (wifi connecting)
void playIdleAnimation(void *)
{
  while (true)
  {
    Controller::get().playIdleAnimation();
    vTaskDelay(WIFI_DELAY);
  }
}

// thread that checks if wifi connected
void checkNetwork(void *)
{
  while (true)
  {
    // if not connected
    if (WiFi.status() != WL_CONNECTED)
    {
      TaskHandle_t animation = NULL;
      LOG("Lost connection\n");
      xTaskCreate(
          playIdleAnimation, // Task function
          "Animation",       // Name of the task (for debugging)
          2000,              // Stack size in words
          NULL,              // Parameter passed to the task
          2,                 // Task priority
          &animation         // Handle to the task
      );
      while (WiFi.status() != WL_CONNECTED)
      {
        vTaskDelay(50);
      }
      // connection established
      LOG("Connected\n");
      vTaskDelete(animation);
    }
    vTaskDelay(100);
  }
}

// main dmx loop
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
  // initialise the contorller
  Controller::get().init2D();

  // create all tasks
  xTaskCreate(dmxLoop, "DMX", 5000, NULL, 3 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(checkNetwork, "Wifi check", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
  xTaskCreate(statReportLoop, "Logging", 2000, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

void loop()
{
  vTaskDelete(NULL);
}
