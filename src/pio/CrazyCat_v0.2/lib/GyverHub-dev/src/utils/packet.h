#pragma once
#include <Arduino.h>

#include "anyText.h"
#include "core/hooks.h"
#include "hub_macro.hpp"
#include "tags.h"

namespace gh::core {

class Packet : public String {
   public:
    Packet(void* hub = nullptr,
         AnswerHook hook = nullptr,
         uint16_t bufsize = 0) : _hub(hub), _hook(hook), _bufsize(bufsize) {}

    void begin() {
        clear();
        concat(F("#{"));
    }
    void end() {
        closeComma();
        concat('#');
    }
    void clear() {
        *(String*)(this) = "";
    }

    // ==================== NUMBER ====================

    template <typename T>
    void addInt(const AnyText& key, T value) {
        addKey(key);
        concat(value);
        _check();
        comma();
    }
    void addFloat(const AnyText& key, float value, uint8_t dec = 2) {
        addKey(key);
        if (isnan(value)) concat('0');
        else {
            if (dec == 2) concat(value);
            else {
                char buf[33];
                dtostrf(value, dec + 2, dec, buf);
                concat(buf);
            }
        }
        _check();
        comma();
    }

    // ==================== STRING ====================

    // строка const char*/FSTR/String
    void addStrEsc(const AnyText& key, const AnyText& value) {
        addKey(key);
        dquot();
        addEscape(value);
        dquot();
        _check();
        comma();
    }

    void addStr(const AnyText& key, const AnyText& value) {
        addKey(key);
        if (value.tag != Tag::None) {
            addCode((uint8_t)value.tag);
        } else {
            dquot();
            if (value.pgm) concat((FSTR)value.str);
            else concat(value.str);
            dquot();
        }
        _check();
        comma();
    }

    // ==================== ADD ====================
    void add(const AnyText& text) {
        if (text.pgm) concat((FSTR)text.str);
        else concat(text.str);
        _check();
    }

    // ==================== MISC ====================
    void addID(const char* id) {
        addStr(Tag::id, id);
    }
    void addClient(const char* id) {
        addStr(Tag::client, id);
    }

    void addKey(const AnyText& key) {
        if (key.tag != Tag::None) {
            addCode((uint8_t)key.tag);
        } else {
            dquot();
            if (key.pgm) concat((FSTR)key.str);
            else concat(key.str);
            dquot();
        }
        colon();
    }

    void addCode(uint8_t tag) {
        concat('#');
        if (tag >= 16) concat(_i16(tag >> 4));
        concat(_i16(tag));
    }

    // ==================== SINGLE ====================
    void quot() {
        concat('\'');
    }
    void dquot() {
        concat('\"');
    }
    void comma() {
        concat(',');
    }
    void colon() {
        concat(':');
    }
    void open() {
        concat('{');
    }
    void openSq() {
        concat('[');
    }
    void close() {
        concat('}');
    }
    void closeSq() {
        concat(']');
    }

    void replaceComma(char c) {
        if (length() > 0 && charAt(length() - 1) == ',') setCharAt(length() - 1, c);
        else concat(c);
    }
    void closeComma() {
        replaceComma('}');
    }
    void closeSqComma() {
        replaceComma(']');
    }

    // ==================== ESCAPE ====================
    void addEscape(const AnyText& str) {
        if (str.pgm) addEscape((FSTR)str.str);
        else addEscape((PGM_P)str.str);
    }

    void addEscape(PGM_P cstr, char sym = '\"') {
        if (!cstr) return;
        if (strchr(cstr, sym)) {
            reserve(length() + strlen(cstr));
            const char* p = cstr;
            while (*p) {
                if (*p == sym && (p == cstr || p[-1] != '\\')) concat('\\');
                concat(*p);
                p++;
            }
        } else {
            concat(cstr);
        }
    }

    void addEscape(FSTR pstr, char sym = '\"') {
        if (!pstr) return;
        PGM_P cstr = (PGM_P)pstr;
        uint16_t len = strlen_P(cstr);
#ifdef ESP32
        if (memchr(cstr, sym, len)) {
#else
        if (memchr_P(cstr, sym, len)) {
#endif
            reserve(length() + len);
            char c = 0;
            bool esc = 0;
            do {
                esc = (c == '\\');
                c = pgm_read_byte(cstr);
                if (c == sym && !esc) concat('\\');
                concat(c);
                cstr++;
            } while (c);
        } else {
            concat(pstr);
        }
    }

    void sendNow() {
        if (_hub) {
            _hook(_hub, *((String*)this), false);
            clear();
        }
    }

    void sendHook(AnyText text) {
        if (_hub) _hook(_hub, text, false);
    }

    bool hasHook() {
        return _hub;
    }

    // ==================== PRIVATE ====================
   private:
    void* _hub;
    AnswerHook _hook;
    uint16_t _bufsize;

    void _check() {
        if (_hub && _bufsize && length() >= _bufsize) sendNow();
    }

    char _i16(uint8_t i) {
        i &= 0xf;
        return (char)(i >= 10 ? (i + 87) : (i + 48));
    }
};

}  // namespace gh::core