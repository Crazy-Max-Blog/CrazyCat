#pragma once
#include <Arduino.h>

#include "hub_macro.hpp"

#ifdef GH_ESP_BUILD
#include <WebSocketsServer.h>

#include "core/core_class.h"
#include "core/hooks.h"
#include "core/hub_class.h"
#include "core/types.h"
#include "utils/anyText.h"

namespace gh::core {
class WS {
    friend class HubCore;
    friend class ::GyverHub;

   private:
    WS() : ws(GH_WS_PORT, "", "hub") {}

    void setup(void* hub, ParseHook prh, FocusHook fch) {
        this->hubptr = hub;
        this->parse_h = prh;
        this->focus_h = fch;
    }

    void begin() {
        ws.onEvent([this](uint8_t num, WStype_t type, uint8_t* data, GH_UNUSED size_t len) {
            switch (type) {
                case WStype_CONNECTED:
                    focus_h(hubptr, gh::Connection::WS, true);
                    break;

                case WStype_DISCONNECTED:
                    focus_h(hubptr, gh::Connection::WS, false);
                    break;

                case WStype_ERROR:
                    break;

                case WStype_TEXT:
                    clientID = num;
                    // parse_h(hubptr, (char*)data, nullptr, gh::Connection::WS, gh::Source::ESP);

                    // TODO
                    // if (len > 3 && data[len - 3] == '/' && data[len - 2] == 'u' && data[len - 1] == 'i') {
                    //     if (url_buf) delete url_buf;
                    //     url_buf = strdup((char*)data);
                    //     url_id = num;
                    // } else {
                    //     parse_h(hubptr, (char*)data, nullptr, gh::Connection::WS, gh::Source::ESP);
                    // }

                    if (url_buf) delete url_buf;
                    url_buf = strdup((char*)data);
                    url_id = num;
                    break;

                default:
                    break;
            }
        });

        ws.begin();
    }

    void end() {
        ws.close();
    }

    void tick() {
        ws.loop();

        if (url_buf) {
            clientID = url_id;
            parse_h(hubptr, url_buf, nullptr, gh::Connection::WS, gh::Source::ESP);
            delete url_buf;
            url_buf = nullptr;
        }
    }

    void send(gh::core::AnyText& text) {
        if (text.pgm) {
            uint8_t buf[text.len];
            memcpy_P(buf, text.str, text.len);
            ws.broadcastTXT(buf, text.len);
        } else {
            ws.broadcastTXT(text.str, text.len);
        }
        yield();
    }

    void answer(gh::core::AnyText& text) {
        if (text.pgm) {
            uint8_t buf[text.len];
            memcpy_P(buf, text.str, text.len);
            ws.sendTXT(clientID, buf, text.len);
        } else {
            ws.sendTXT(clientID, text.str, text.len);
        }
        yield();
    }

    // ======================= PRIVATE =======================
    WebSocketsServer ws;
    uint8_t clientID = 0;
    void* hubptr;
    ParseHook parse_h;
    FocusHook focus_h;

    char* url_buf = nullptr;
    uint8_t url_id;
};
}  // namespace gh::core
#endif