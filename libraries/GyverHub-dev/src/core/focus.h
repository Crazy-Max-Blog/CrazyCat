#pragma once
#include <Arduino.h>

#include "core/types.h"

namespace gh::core {

class Focus {
   public:
    void set(gh::Connection connection) {
        write(connection, 1);
    }

    void clear(gh::Connection connection) {
        write(connection, 0);
    }

    void write(gh::Connection connection, bool focus) {
        _arr[(uint8_t)connection] = focus ? GH_CONN_TOUT : 0;
    }

    void tick() {
        if ((uint16_t)((uint16_t)millis() - _focus_tmr) >= 1000) {
            _focus_tmr = millis();
            for (uint8_t i = 0; i < GH_CONN_AMOUNT; i++) {
                if (_arr[i]) _arr[i]--;
            }
        }
    }

    bool focused() {
        for (uint8_t i = 0; i < GH_CONN_AMOUNT; i++) {
            if (_arr[i]) return 1;
        }
        return 0;
    }
    bool focused(gh::Connection connection) {
        return _arr[(uint8_t)connection];
    }

   private:
    uint8_t _arr[GH_CONN_AMOUNT] = {0};
    uint16_t _focus_tmr = 0;
};

}  // namespace gh::core