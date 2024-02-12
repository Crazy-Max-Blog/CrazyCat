#pragma once
#include <Arduino.h>

namespace gh::core {

uint32_t crc32(uint32_t crc, uint8_t* buffer, uint32_t len, bool pgm = 0);

}