#pragma once
#include <Arduino.h>

#include "tags.h"

namespace gh::core {

// строка в любом виде (const char*, char*, String, PROGMEM)
class AnyText {
   public:
    AnyText() {}
    AnyText(const String& s) : str(s.c_str()), len(s.length()) {}
    AnyText(const char* s, uint32_t len = 0) : str(s), len(len) {}
    AnyText(Tag tag) : tag(tag) {}
    AnyText(const __FlashStringHelper* s, uint32_t len = 0) : str((const char*)s), len(len), pgm(1) {}

    void calcLen() {
        if (!len) {
            if (pgm) len = strlen_P(str);
            else len = strlen(str);
        }
    }

    const char* str = nullptr;
    uint32_t len = 0;
    bool pgm = 0;
    Tag tag = Tag::None;

   private:
};

}  // namespace gh::core