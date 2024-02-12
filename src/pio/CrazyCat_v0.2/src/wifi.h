#pragma once

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "config.h"

//#include "ghub.h"

void wifi_begin() {
#ifdef ESP_BUILD
    // ESP
    // подключение к роутеру
    WiFi.mode(WIFI_STA);
    WiFi.begin(sets.wifi.ssid, sets.wifi.pass);
    int c=0;
    while (WiFi.status() != WL_CONNECTED and c<=15) {
        c++;
        delay(500);
        Serial.print(".");
    }
    if(WiFi.status() != WL_CONNECTED) {WiFi.disconnect();WiFi.softAP("ESP");}
    Serial.println();
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.softAPIP());
    
    //mqtt_reconnect();
//#elif
    //AVR
#endif
}
