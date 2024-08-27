#include "HttpSwitcher.h"

void setup()
{
  Serial.println("setup");
  Serial.begin(115200);
  delay(5000);
  start();
  Serial.println("initialized");
}

void loop()
{
  tick();
}
