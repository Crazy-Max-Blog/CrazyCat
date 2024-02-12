#include "crc32.h"

namespace gh::core {

uint32_t crc32(uint32_t crc, uint8_t* buffer, uint32_t len, bool pgm) {
    crc = ~crc;
    while (len--) {
        if (pgm) crc ^= pgm_read_byte(buffer++);
        else crc ^= *buffer++;
        for (uint8_t i = 0; i < 8; i++) crc = (crc & 1) ? ((crc >> 1) ^ 0x4C11DB7) : (crc >> 1);
    }
    return ~crc;
}

}  // namespace gh::core