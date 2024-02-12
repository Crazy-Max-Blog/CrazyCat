#pragma once
#include <Arduino.h>

namespace gh::b64 {

char getChar(uint8_t n);
uint8_t getByte(char b);
uint16_t encodedLen(uint16_t len);
uint16_t decodedLen(const char* data, uint16_t len);
void encode(String* out, uint8_t* data, uint16_t len, bool pgm = false);
void decode(uint8_t* out, const char* data, uint16_t len);

}  // namespace gh::b64