#include "protocol_v1.h"

uint16_t protocol_v1_crc16_ccitt_false(const uint8_t *data, uint32_t len) {
    uint16_t crc = 0xFFFFu;
    uint32_t i;

    for (i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000u) {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}
