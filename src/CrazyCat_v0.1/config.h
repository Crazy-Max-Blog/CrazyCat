//#pragma once

#include <Arduino.h>

#if (defined(ESP8266) || defined(ESP32))
#define ESP_BUILD
#endif

#ifdef GH_EESP_BUILD
#include <FileData.h>
#include <LittleFS.h>
#else
#include "EEPROM.h"
#endif

int minvt = 100;

#define PR_NAME    "CrazyCat"
#define PR_ACCOUNT "Crazy-Max-Blog"
#define PR_VERSION "0.1"

#define S String

#define LED_PIN sets.devs.led.pin

struct Settings {
  struct {
    struct {
      int pin;
    } led;
  } devs;
#ifdef ESP_BUILD
  struct {
    char ssid[25]="";
    char pass[25]="";
  } wifi;
  struct {
    char host[25]="";
    char port[10]="";
    char ssid[25]="";
    char pass[25]="";
  } mqtt;
#endif
};

Settings sets;

#ifdef ESP_BUILD
FileData data(&LittleFS, "/data.dat", 'B', &sets, sizeof(sets));
#endif

void data_begin() {
#ifdef GH_EESP_BUILD
  LittleFS.begin();
  FDstat_t stat = data.read();
#else
  EEPROM.begin();
  EEPROM.get(0, sets);
#endif
}

void data_save() {
#ifdef ESP_BUILD
  data.updateNow();
#else
  EEPROM.put(0, sets);
#endif
}