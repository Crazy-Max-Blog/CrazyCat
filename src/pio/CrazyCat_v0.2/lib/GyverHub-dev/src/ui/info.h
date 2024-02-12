#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/core_class.h"
#include "core/fs.h"
#include "hub_macro.hpp"
#include "utils/anyText.h"
#include "utils/packet.h"

#ifdef GH_ESP_BUILD
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#endif
#endif  // GH_ESP_BUILD

namespace gh {

class Info;
namespace core {
typedef void (*InfoCallback)(gh::Info& info);
}

class Info {
    friend class gh::core::HubCore;

   public:
    // тип info
    enum class Type : uint8_t {
        Version,
        Network,
        Memory,
        System,
    };

    Info(gh::core::Packet* answ, Type type, gh::Client* client) : client(*client), type(type), answ(answ) {}

    // добавить текстовое поле в info
    void addStr(gh::core::AnyText label, gh::core::AnyText text) {
        answ->addStrEsc(label, text);
    }

    // добавить целочисленное поле в info
    template <typename T>
    void addInt(gh::core::AnyText label, T val) {
        answ->addInt(label, val);
    }

    // добавить дробное поле в info
    void addFloat(gh::core::AnyText label, float val, uint8_t dec = 2) {
        answ->addFloat(label, val, dec);
    }

    gh::Client client;
    Type type;

    // =========================== PRIVATE ===========================
   private:
    gh::core::Packet* answ;

    static void _buildGroup(gh::core::InfoCallback cb, gh::core::Packet& answ, Type type, gh::Client* client) {
        if (cb) {
            Info info(&answ, type, client);
            cb(info);
        }
    }

    static void _build(gh::core::InfoCallback cb, gh::core::Packet& answ, String* version, gh::Client* client) {
        // ================ VERSION ================
        answ += F("\"info\":{\"version\":{\"Library\":\"" GH_LIB_VERSION "\",");
        if (version->length()) answ.addStr(F("Firmware"), *version);
        _buildGroup(cb, answ, Type::Version, client);
        answ.closeComma();

        // ================ NETWORK ================
        answ += (F(",\"net\":{"));
#ifdef GH_ESP_BUILD
        switch (WiFi.getMode()) {
            case WIFI_AP:
                answ.addStr(F("Mode"), F("AP"));
                answ.addStr(F("AP_IP"), WiFi.softAPIP().toString());
                answ.addStr(F("AP_SSID"), WiFi.softAPSSID());
                answ.addStr(F("AP_MAC"), WiFi.softAPmacAddress());
                break;

            case WIFI_STA:
                answ.addStr(F("Mode"), F("STA"));
                answ.addStr(F("IP"), WiFi.localIP().toString());
                answ.addStr(F("SSID"), WiFi.SSID());
                answ.addStr(F("MAC"), WiFi.macAddress());
                break;

            case WIFI_AP_STA:
                answ.addStr(F("Mode"), F("AP+STA"));
                answ.addStr(F("AP_IP"), WiFi.softAPIP().toString());
                answ.addStr(F("AP_SSID"), WiFi.softAPSSID());
                answ.addStr(F("AP_MAC"), WiFi.softAPmacAddress());
                answ.addStr(F("IP"), WiFi.localIP().toString());
                answ.addStr(F("SSID"), WiFi.SSID());
                answ.addStr(F("MAC"), WiFi.macAddress());
                break;

            default:
                break;
        }
        answ.addStr(F("RSSI"), String(constrain(2 * (WiFi.RSSI() + 100), 0, 100)) + '%');
#endif
        _buildGroup(cb, answ, Type::Network, client);
        answ.closeComma();

        // ================ MEMORY ================
        answ += (F(",\"memory\":{"));
#ifdef GH_ESP_BUILD
        answ.addInt(F("RAM"), String("[") + ESP.getFreeHeap() + ",0]");

#ifndef GH_NO_FS
        answ.addInt(F("Flash"), String("[") + gh::FS.used() + ',' + gh::FS.totalSpace() + "]");
        answ.addInt(F("Sketch"), String("[") + ESP.getSketchSize() + ',' + ESP.getFreeSketchSpace() + "]");
#endif
#endif
        _buildGroup(cb, answ, Type::Memory, client);
        answ.closeComma();

        // ================ SYSTEM ================
        answ += (F(",\"system\":{"));
        answ.addInt(F("Uptime"), millis() / 1000ul);
        answ.addStr(F("Platform"), GH_PLATFORM);

#ifdef GH_ESP_BUILD
        answ.addInt(F("CPU_MHz"), ESP.getCpuFreqMHz());
        answ.addStr(F("Flash_chip"), String(ESP.getFlashChipSize() / 1000.0, 1) + " kB");
#endif
        _buildGroup(cb, answ, Type::System, client);
        answ.closeComma();
        answ.close();
    }
};

}  // namespace gh