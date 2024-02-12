#pragma once

#define GH_INCLUDE_PORTAL
#include <GyverHub.h>
GyverHub hub;

#include "config.h"

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

  b.beginRow();
  b.Icon_("led", &ghub_led).label("Cat").icon("").noTab();
  b.Slider(&sets.devs.tenz.min_mass).label("Min").range(0,1000,10).noTab().size(4);
  b.endRow();
  
  //b.addJSON_File("ui.json");
}

void gh_sets(GH::Builder& b) {
  b.Label().valueStr("Devices").noLabel(1).noTab(1);
  
  b.Label().valueStr("Led").noLabel(1).noTab(1);
  b.Spinner(&sets.devs.led.pin).label("Pin").noTab(1).click();
  if(b.Button().label("Save and Update").noTab(1).click())    {data_save();led_begin();}
  
  b.Label().valueStr("Tenzo").noLabel(1).noTab(1);
  b.Slider(&sets.devs.tenz.min_mass).label("Min").range(0,1000,10).noTab();
  b.beginRow();
  b.Spinner(&sets.devs.tenz.pin_data).label("Data pin").noTab(1).click();
  b.Spinner(&sets.devs.tenz.pin_clk).label("Clock pin").noTab(1).click();
  b.endRow();
  if(b.Button().label("Save and Update").noTab(1).click())    {data_save();tenz_begin();}

  
#ifdef ESP_BUILD
  b.Label().valueStr("WiFi").noLabel(1).noTab(1).noTab(1);
  b.Input(sets.wifi.ssid).label("Ssid").noTab(1).click();                           
  b.Pass(sets.wifi.pass ).label("Password").noTab(1).click();                       
  if(b.Button().label("Save and Restart").noTab(1).click())   {data_save();ESP.restart();}
  
  b.Label().valueStr("MQTT").noLabel(1).noTab(1);
  b.Input_("as1", sets.mqtt.host).label("Host").noTab(1).click();                           
  b.Input_("ds2", sets.mqtt.port).label("Port").noTab(1).click();                           
  b.Input_("vf3", sets.mqtt.ssid).label("Login").noTab(1).click();                          
  b.Pass(sets.mqtt.pass).label("Password").noTab(1).click();                        
  
  //if(b.Button().label("Save").noTab(1).click())               {data_save();}
  //if(b.Button().label("Reconnect").noTab(1).click())          {mqtt_reconnect();}
  if(b.Button().label("Save and Reconnect").noTab(1).click()) {data_save();mqtt_reconnect();}
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
    
//#ifdef PR_USE_SERIAL_HUB
  // подключить экземпляр любого Stream-класса
  hub.stream.config(&Serial, gh::Connection::Serial);
//#endif

#ifdef ESP_BUILD
  if(WiFi.status() == WL_CONNECTED) 
    mqtt_reconnect(); //hub.mqtt.config(sets.mqtt.host, sets.mqtt.port.toInt(), sets.mqtt.ssid, sets.mqtt.pass);  // + MQTT
#endif

  hub.config(F(PR_PREFIX), F(PR_NAME), PR_ICON, 0x55443420);
  hub.onBuild(build);
  hub.setVersion((S)F(PR_ACCOUNT)+(S)F("/")+(S)F(PR_NAME)+(S)F("@")+(S)F(PR_VERSION));
  hub.begin();
}

void ghub_tick() {
  hub.tick();
  //data.tick();
  //static gh::Timer tmr(500);
  //if(tmr) hub.update("mass").valueInt(random(10));
}

void mqtt_reconnect() {
#ifdef ESP_BUILD
  int mport = String(sets.mqtt.port).toInt();
  if(WiFi.status() == WL_CONNECTED)     hub.mqtt.config(sets.mqtt.host, mport, sets.mqtt.ssid, sets.mqtt.pass);  // + MQTT
#endif
}
