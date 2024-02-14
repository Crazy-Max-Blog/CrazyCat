#pragma once
#include <GyverHX711.h>
GyverHX711 sensor(sets.devs.tenz.pin_data, sets.devs.tenz.pin_clk, HX_GAIN64_A);
int tenz_valuet;

//#include "ghub.h"

void tenz_begin() {
  sensor.set_pins(sets.devs.tenz.pin_data, sets.devs.tenz.pin_clk);
  delay(500);
  sensor.tare();    // калибровка нуля
}

void tenz_tick() {
  static long timer;
  if (sensor.available() && millis()-timer>10) {
    timer=millis();
    tenz_valuet=int(sensor.read()/10+5);
  }
}
