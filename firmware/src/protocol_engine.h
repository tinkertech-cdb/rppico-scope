#ifndef PROTOCOL_ENGINE_H
#define PROTOCOL_ENGINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol_v1.h"

typedef struct {
    protocol_v1_scope_cfg_t scope_cfg;
    protocol_v1_fg_cfg_t fg_cfg;
    bool streaming;
    uint32_t frame_id;
} protocol_engine_state_t;

void protocol_engine_init(protocol_engine_state_t *state);

size_t protocol_engine_handle_packet(protocol_engine_state_t *state,
                                     const uint8_t *in_packet,
                                     size_t in_len,
                                     uint8_t *out_packet,
                                     size_t out_capacity);

#endif
