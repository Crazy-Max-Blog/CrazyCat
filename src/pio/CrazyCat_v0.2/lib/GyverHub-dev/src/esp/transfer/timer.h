#pragma once
#include <Arduino.h>

namespace gh::core {
class Timer {
   public:
    // запустить
    void start() {
        tmr = millis();
        if (!tmr) tmr = 1;
    }

    // остановить
    void stop() {
        tmr = 0;
    }

    // сработал
    bool tick(uint16_t prd) {
        return (tmr && (uint16_t)((uint16_t)millis() - tmr) >= prd) ? (stop(), true) : false;
    }

   private:
    uint16_t tmr = 0;
};
}  // namespace gh::core