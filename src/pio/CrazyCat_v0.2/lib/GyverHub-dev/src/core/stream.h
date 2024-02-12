#pragma once
#include <Arduino.h>
#include <Stream.h>

#include "core/core_class.h"
#include "core/hooks.h"
#include "core/hub_class.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "utils/anyText.h"

namespace gh::core {

class HubStream {
    friend class HubCore;
    friend class ::GyverHub;

   public:
    // настроить Stream
    void config(Stream* stream, gh::Connection conn) {
        this->stream = stream;
        this->conn = conn;
    }

    // отключить
    void stop() {
        this->stream = nullptr;
    }

    // ======================= PRIVATE =======================
   private:
    void setup(void* hub, ParseHook prh) {
        this->hubptr = hub;
        this->parse_h = prh;
    }

    void tick() {
        if (stream && stream->available()) {
            String str = stream->readStringUntil('\0');
            parse_h(hubptr, (char*)str.c_str(), nullptr, conn, gh::Source::Stream);
        }
    }

    void send(gh::core::AnyText& text) {
        if (stream) {
            if (text.pgm) stream->print((FSTR)text.str);
            else stream->print(text.str);
        }
    }

    bool state() {
        return stream;
    }

    gh::Connection connection() {
        return conn;
    }

    // ======================= PRIVATE =======================
    Stream* stream = nullptr;
    gh::Connection conn;
    void* hubptr;
    ParseHook parse_h;
};

}  // namespace gh::core