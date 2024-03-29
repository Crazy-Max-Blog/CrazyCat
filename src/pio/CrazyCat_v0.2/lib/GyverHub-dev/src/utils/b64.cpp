#include "b64.h"

namespace gh::b64 {
static const char _getChars[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8_t _b64index[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0,
    0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

char getChar(uint8_t n) {
    return pgm_read_byte(_getChars + n);
}
uint8_t getByte(char b) {
    return pgm_read_byte(_b64index + b);
}

uint16_t encodedLen(uint16_t len) {
    return ((len + 3 - 1) / 3) * 4;  // padded
}

uint16_t decodedLen(const char* data, uint16_t len) {
    if (len < 4) return 0;
    uint8_t padd = 0;
    if (data[len - 2] == '=') padd = 2;
    else if (data[len - 1] == '=') padd = 1;
    return ((len + 3) / 4) * 3 - padd;
}

void encode(String* out, uint8_t* data, uint16_t len, bool pgm) {
    uint16_t slen = out->length();
    out->reserve(slen + encodedLen(len));
    int8_t valb = -6;
    uint16_t val = 0;
    for (uint16_t i = 0; i < len; i++) {
        val = (val << 8) + (pgm ? pgm_read_byte(&data[i]) : data[i]);
        valb += 8;
        while (valb >= 0) {
            *out += getChar((val >> valb) & 0x3F);
            valb -= 6;
        }
    }
    if (valb > -6) *out += getChar(((val << 8) >> (valb + 8)) & 0x3F);
    while ((out->length() - slen) % 4) *out += '=';
}

void decode(uint8_t* out, const char* data, uint16_t len) {
    if (!decodedLen(data, len)) return;
    uint16_t val = 0, idx = 0;
    int8_t valb = -8;
    for (uint16_t i = 0; i < len; i++) {
        if (data[i] == '=') break;
        val = (val << 6) + getByte(data[i]);
        valb += 6;
        if (valb >= 0) {
            out[idx++] = (uint8_t)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
}

}  // namespace gh::b64