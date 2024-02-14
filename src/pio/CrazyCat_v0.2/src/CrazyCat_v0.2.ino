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
  Serial.println(11);
  wifi_begin();
  Serial.println(12);
  ghub_begin();
  Serial.println(13);
  tenz_begin();
  Serial.println(14);
  auto_begin();
  Serial.println(15);
  led_begin();
  Serial.println(16);
}

void loop() {
  ghub_tick();
  Serial.println(1);
  tenz_tick();
  Serial.println(2);
  auto_tick();
  Serial.println(3);
  led_tick();
  Serial.println(4);
}
