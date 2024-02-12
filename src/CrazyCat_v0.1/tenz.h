#include <GyverHX711.h>
GyverHX711 sensor(3, 2, HX_GAIN64_A);
int tenz_valuet;

//#include "ghub.h"

void tenz_begin() {
  //delay(500);
  sensor.tare();    // калибровка нуля
}

void tenz_tick() {
  static long timer;
  if (sensor.available() && millis()-timer>10) {
    timer=millis();
    tenz_valuet=int(sensor.read()/10+5);
  }
}
