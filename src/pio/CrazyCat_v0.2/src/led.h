#pragma once
#include "config.h"
void led_es(bool s);
bool led_state = 0;

void led_begin() {
  pinMode(LED_PIN, OUTPUT);
}

void led_tick() {
  static bool bbb = 0;
  if(bbb!=led_state) {bbb=led_state;led_es(led_state);}
}

void led_es(bool s) {
  //Serial.println(s);
  digitalWrite(LED_PIN, s);
}