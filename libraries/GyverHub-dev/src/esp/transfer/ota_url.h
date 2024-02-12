#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "hub_macro.hpp"

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#else
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#endif

namespace gh::core {
class OtaUrl {
   public:
    OtaUrl(const char* url, const char* type, gh::Client& client) : client(client), url(url) {
        fs = !strcmp(type, "fs");
    }

    bool update() {
#ifdef ESP8266
        ESPhttpUpdate.rebootOnUpdate(false);
        BearSSL::WiFiClientSecure client;
        if (url.startsWith(F("https"))) client.setInsecure();
        if (fs) return ESPhttpUpdate.updateFS(client, url);
        else return ESPhttpUpdate.update(client, url);
#else
        httpUpdate.rebootOnUpdate(false);
        WiFiClientSecure client;
        if (url.startsWith(F("https"))) client.setInsecure();
        if (fs) return httpUpdate.updateSpiffs(client, url);
        else return httpUpdate.update(client, url);
#endif
    }

    gh::Client client;

   private:
    String url;
    bool fs;
};
}  // namespace gh::core