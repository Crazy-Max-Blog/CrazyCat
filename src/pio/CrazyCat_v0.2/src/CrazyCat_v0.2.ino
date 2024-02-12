//#include "Arduino.h"

#include "config.h"
#include "tenz.h"
#include "led.h"
#include "wifi.h"
#include "ghub.h"
#include "auto.h"

void setup() {
  Serial.begin(115200);
  data_begin();
  wifi_begin();
  ghub_begin();
  tenz_begin();
  auto_begin();
  led_begin();
}

void loop() {
  ghub_tick();
  tenz_tick();
  auto_tick();
  led_tick();
}
