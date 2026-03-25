#include <Arduino.h>
#include <LCD_Control.h>

// Pointer to the controller instance
LCD_Control *lcdctrl = nullptr;

void setup()
{
  // Start serial for debugging
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
    Serial.println("Waiting for Serial...");
  };
  Serial.println("System Initializing...");

  // Create the LCD controller instance
  lcdctrl = new LCD_Control();
  lcdctrl->labelDisplays(); // label displays with their addresses for easy identification   
  Serial.println("Initialization complete.");
  Serial.println(String(lcdctrl->getLcds().size()) + " LCD(s) detected.");
}

void loop()
{
  static unsigned long lastsecs = 0;
  unsigned long secs = millis() / 1000;

  if (secs != lastsecs)
  {
    lastsecs = secs;
    for (auto &dev : lcdctrl->getLcds())
    {
      // helper defined below
      unsigned long elapsed = secs;
      unsigned int hr = elapsed / 3600;
      unsigned int mins = (elapsed / 60) % 60;
      unsigned int sec = elapsed % 60;
      char buf[16];
      snprintf(buf, sizeof(buf), "%02u:%02u:%02u", hr, mins, sec);
      dev.printAligned(1, String(buf), LCD_Control::Align::Center);
    }
  }
}