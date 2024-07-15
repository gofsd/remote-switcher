#include "HttpSwitcher.h"

void setup()
{
  Serial.begin(9600);
  start();
  Serial.println("initialized");
}

void loop()
{
  tick();
}
