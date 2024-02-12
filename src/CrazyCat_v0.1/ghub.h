#pragma once

#define GH_INCLUDE_PORTAL
#include <GyverHub.h>
GyverHub hub;

//#include "tenzo.h"

//#define GH::core::AnyPtr GH::core::AnyPtr

bool ghub_led = 0;

void mqtt_reconnect();

void gh_home(GH::Builder& b) {
  b.Title().valueStr("Home").noLabel(1).noTab(1);
  if(b.beginRow()) {
    b.Label().valueStr("Mass:" ).noTab().noLabel();
    b.Label_("mass").valueInt(tenz_valuet).noTab().noLabel();

    if(b.Button().label("Tare").noTab().click()) sensor.tare();
    bool st = true;
    if(b.Switch(&st).label("State").noTab().click()) sensor.sleepMode(!st);
    b.endRow();
  }

  b.Slider(&minvt).label("Min").range(0,1000,10).noTab();
  b.LED_("led", &ghub_led).label("Cat").noTab();
  
  //b.addJSON_File("ui.json");
}

void gh_sets(GH::Builder& b) {
  b.Label().valueStr("Devices").noLabel(1).noTab(1);
  
  b.Label().valueStr("Led").noLabel(1).noTab(1);
  b.Spinner(&sets.devs.led.pin).label("Pin").noTab(1).click();                           //)      //Serial.println(sets.led.pin);
  if(b.Button().label("Save and Update").noTab(1).click())    {data_save();pinMode(sets.devs.led.pin, OUTPUT);}

  
#ifdef ESP_BUILD
  b.Label().valueStr("WiFi").noLabel(1).noTab(1).noTab(1);
  b.Input(sets.wifi.ssid).label("Ssid").noTab(1).click();                           //)      //Serial.println((String)sets.wifi.ssid);
  b.Pass(sets.wifi.pass ).label("Password").noTab(1).click();                       //)  //Serial.println((String)sets.wifi.pass);
  if(b.Button().label("Save and Restart").noTab(1).click())   {data_save();ESP.restart();}
  
  b.Label().valueStr("MQTT").noLabel(1).noTab(1);
  b.Input_("as1", sets.mqtt.host).label("Host").noTab(1).click();                           //)      //Serial.println((String)sets.mqtt.host);
  b.Input_("ds2", sets.mqtt.port).label("Port").noTab(1).click();                           //)      //Serial.println((String)sets.mqtt.port);
  b.Input_("vf3", sets.mqtt.ssid).label("Login").noTab(1).click();                          //)     //Serial.println((String)sets.mqtt.ssid);
  b.Pass(sets.mqtt.pass).label("Password").noTab(1).click();                        //)   //Serial.println((String)sets.mqtt.pass);
  
  //if(b.Button().label("Save").noTab(1).click())               {data_save();}
  //if(b.Button().label("Reconnect").noTab(1).click())          {mqtt_reconnect();}
  if(b.Button().label("Save and Reconnect").noTab(1).click()) {data_save();mqtt_reconnect();}
  //if(b.Button().label("Save and Reconnect").click()) {hub.mqtt.config(sets.mqtt.host, sets.mqtt.port.toInt(), sets.mqtt.ssid, sets.mqtt.pass);}
#endif
}


// билдер
void build(GH::Builder& b) {
  b.Menu("Home;Settings");
  switch (b.menu()) {
    case 0:
      gh_home(b);
      break;
    case 1:
      gh_sets(b);
      break;
    default:
      break;
  }
}

void ghub_begin() {    
    
  // подключить экземпляр любого Stream-класса
  hub.stream.config(&Serial, gh::Connection::Serial);
  //if(WiFi.status() == WL_CONNECTED) 
  //  mqtt_reconnect(); //hub.mqtt.config(sets.mqtt.host, sets.mqtt.port.toInt(), sets.mqtt.ssid, sets.mqtt.pass);  // + MQTT
  
  hub.config(F("MyDevices2"), F("CrazyCat"), "", 0x55443420);
  hub.onBuild(build);
  hub.setVersion((S)F(PR_ACCOUNT)+(S)F("/")+(S)F(PR_NAME)+(S)F("@")+(S)F(PR_VERSION));
  hub.begin();
}

void ghub_tick() {
  hub.tick();
  //data.tick();
  static gh::Timer tmr(500);
  //if(tmr) hub.update("mass").valueInt(random(10));
}

void mqtt_reconnect() {
#ifdef ESP_BUILD
  int mport = String(sets.mqtt.port).toInt();
  if(WiFi.status() == WL_CONNECTED)     hub.mqtt.config(sets.mqtt.host, mport, sets.mqtt.ssid, sets.mqtt.pass);  // + MQTT
#endif
}
