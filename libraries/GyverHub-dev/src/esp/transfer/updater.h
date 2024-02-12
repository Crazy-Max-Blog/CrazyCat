#pragma once
#include <Arduino.h>

#include "core/client.h"
#include "core/types.h"
#include "hub_macro.hpp"
#include "timer.h"
#include "utils/b64.h"

#ifdef ESP8266
#include <Updater.h>
#else
#include <Update.h>
#endif

namespace gh::core {

using gh::Error;

class Updater {
   public:
    Updater(gh::Client& client,
            gh::Reboot* reason) : client(client),
                                  reason(reason) {}

    bool begin(const char* type) {
        uint8_t ota_type = 0;
        if (!strcmp_P(type, PSTR("flash"))) ota_type = 1;
        else if (!strcmp_P(type, PSTR("fs"))) ota_type = 2;

        if (ota_type) {
            size_t ota_size = 0;
            if (ota_type == 1) {
                ota_type = U_FLASH;
#ifdef ESP8266
                ota_size = (size_t)((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
#else
                ota_size = UPDATE_SIZE_UNKNOWN;
#endif
            } else {
#ifdef ESP8266
                ota_type = U_FS;
                close_all_fs();
#ifndef GH_NO_FS
                ota_size = (size_t)&_FS_end - (size_t)&_FS_start;
#endif
#else
                ota_type = U_SPIFFS;
                ota_size = UPDATE_SIZE_UNKNOWN;
#endif
            }
            if (!::Update.begin(ota_size, ota_type)) {
                setError(Error::Start);
            }
        }
        return !hasError();
    }

    bool write64(const char* data) {
        if (hasError()) return 0;
        uint16_t len = strlen(data);
        uint16_t declen = gh::b64::decodedLen(data, len);
        uint8_t buf[declen];
        gh::b64::decode(buf, data, len);
        return write(buf, declen);
    }

    bool write(uint8_t* data, uint32_t len) {
        tmr.start();
        if (hasError()) return 0;
        if (::Update.write(data, len) != len) {
            setError(Error::Write);
        }
        return !hasError();
    }

    bool end() {
        if (hasError()) return 0;
        if (::Update.end(true)) {
            *reason = gh::Reboot::Ota;
        } else {
            setError(Error::End);
        }
        return !hasError();
    }

    bool timeout() {
        if (tmr.tick(GH_CONN_TOUT * 1000)) {
            setError(Error::Timeout);
        }
        return hasError();
    }

    void abort() {
        setError(Error::Abort);
    }

    gh::Error error = gh::Error::None;

    bool hasError() {
        return error != gh::Error::None;
    }

    void setError(gh::Error err) {
        error = err;
    }

    gh::Client client;

   private:
    gh::Reboot* reason;
    Timer tmr;
};

}  // namespace gh::core