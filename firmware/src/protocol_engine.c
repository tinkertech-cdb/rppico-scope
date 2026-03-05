#include "protocol_engine.h"

#include <string.h>

#include "project_config.h"

static const protocol_v1_device_caps_t k_device_caps = {
    .feature_bits = PROTO_FEAT_DUAL_CHANNEL_SCOPE |
                    PROTO_FEAT_FUNCTION_GENERATOR |
                    PROTO_FEAT_TRIGGER_SINGLE |
                    PROTO_FEAT_STREAM_DROP_COUNTER,
    .max_channels = 2,
    .max_adc_ksps_agg = 500,
    .min_sample_rate_hz = 100,
    .max_sample_rate_hz = 500000,
    .max_stream_payload = 2048,
    .fg_freq_min_millihz = 1000,
    .fg_freq_max_millihz = 200000000,
    .fg_amp_min_mvpp = 0,
    .fg_amp_max_mvpp = 3300,
    .fg_offset_min_mv = -1650,
    .fg_offset_max_mv = 1650,
};

static void set_default_scope_cfg(protocol_v1_scope_cfg_t *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->channel_mask = 0x03;
    cfg->run_mode = PROTO_SCOPE_AUTO;
    cfg->trigger_source = PROTO_TRIGGER_CH1;
    cfg->trigger_edge = PROTO_TRIGGER_RISING;
    cfg->trigger_level = 127;
    cfg->pretrigger_pct = 50;
    cfg->sample_rate_hz = 100000;
    cfg->timebase_ps = 1000000000u;
}

static void set_default_fg_cfg(protocol_v1_fg_cfg_t *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->waveform = PROTO_FG_SINE;
    cfg->enabled = 0;
    cfg->frequency_millihz = 1000000;
    cfg->amplitude_mvpp = 1000;
    cfg->offset_mv = 0;
}

void protocol_engine_init(protocol_engine_state_t *state) {
    memset(state, 0, sizeof(*state));
    set_default_scope_cfg(&state->scope_cfg);
    set_default_fg_cfg(&state->fg_cfg);
}

static bool valid_scope_cfg(const protocol_v1_scope_cfg_t *cfg) {
    if ((cfg->channel_mask & 0x03u) == 0) {
        return false;
    }
    if (cfg->run_mode > PROTO_SCOPE_SINGLE) {
        return false;
    }
    if (cfg->trigger_source > PROTO_TRIGGER_CH2) {
        return false;
    }
    if (cfg->trigger_edge > PROTO_TRIGGER_FALLING) {
        return false;
    }
    if (cfg->pretrigger_pct > 90) {
        return false;
    }
    if (cfg->sample_rate_hz < k_device_caps.min_sample_rate_hz ||
        cfg->sample_rate_hz > k_device_caps.max_sample_rate_hz) {
        return false;
    }
    if ((cfg->channel_mask & 0x03u) == 0x03u && cfg->sample_rate_hz > (RPPICO_SAFE_ADC_MAX_AGGREGATE_SPS / 2u)) {
        return false;
    }
    return true;
}

static bool valid_fg_cfg(const protocol_v1_fg_cfg_t *cfg) {
    if (cfg->waveform > PROTO_FG_DC) {
        return false;
    }
    if (cfg->enabled > 1) {
        return false;
    }
    if (cfg->frequency_millihz < k_device_caps.fg_freq_min_millihz ||
        cfg->frequency_millihz > k_device_caps.fg_freq_max_millihz) {
        return false;
    }
    if (cfg->amplitude_mvpp < k_device_caps.fg_amp_min_mvpp ||
        cfg->amplitude_mvpp > k_device_caps.fg_amp_max_mvpp) {
        return false;
    }
    if (cfg->offset_mv < k_device_caps.fg_offset_min_mv ||
        cfg->offset_mv > k_device_caps.fg_offset_max_mv) {
        return false;
    }
    return true;
}

static size_t pack_rsp(uint16_t seq,
                       uint16_t cmd_id,
                       uint16_t status,
                       const void *payload,
                       uint16_t payload_len,
                       uint8_t *out_packet,
                       size_t out_capacity) {
    const size_t total_len = sizeof(protocol_v1_header_t) + sizeof(protocol_v1_rsp_envelope_t) + payload_len;
    protocol_v1_header_t *hdr;
    protocol_v1_rsp_envelope_t *rsp;
    uint16_t crc;

    if (out_capacity < total_len) {
        return 0;
    }

    hdr = (protocol_v1_header_t *)out_packet;
    rsp = (protocol_v1_rsp_envelope_t *)(out_packet + sizeof(protocol_v1_header_t));

    hdr->magic = PROTOCOL_V1_MAGIC;
    hdr->version = PROTOCOL_V1_VERSION;
    hdr->kind = PROTO_KIND_RSP;
    hdr->seq = seq;
    hdr->len = (uint16_t)(sizeof(protocol_v1_rsp_envelope_t) + payload_len);
    hdr->crc16 = 0;

    rsp->cmd_id = cmd_id;
    rsp->status = status;
    if (payload_len > 0 && payload != NULL) {
        memcpy(rsp->payload, payload, payload_len);
    }

    crc = protocol_v1_crc16_ccitt_false(out_packet, (uint32_t)total_len);
    hdr->crc16 = crc;

    return total_len;
}

size_t protocol_engine_handle_packet(protocol_engine_state_t *state,
                                     const uint8_t *in_packet,
                                     size_t in_len,
                                     uint8_t *out_packet,
                                     size_t out_capacity) {
    const protocol_v1_header_t *hdr;
    const protocol_v1_cmd_envelope_t *cmd;
    const uint8_t *cmd_payload;
    uint16_t cmd_payload_len;

    if (in_packet == NULL || out_packet == NULL || in_len < sizeof(protocol_v1_header_t) + sizeof(protocol_v1_cmd_envelope_t)) {
        return 0;
    }

    hdr = (const protocol_v1_header_t *)in_packet;
    if (hdr->magic != PROTOCOL_V1_MAGIC || hdr->version != PROTOCOL_V1_VERSION || hdr->kind != PROTO_KIND_CMD) {
        return 0;
    }

    if (sizeof(protocol_v1_header_t) + hdr->len != in_len) {
        return 0;
    }

    {
        uint16_t computed_crc;
        uint8_t temp[512];
        if (in_len > sizeof(temp)) {
            return 0;
        }
        memcpy(temp, in_packet, in_len);
        ((protocol_v1_header_t *)temp)->crc16 = 0;
        computed_crc = protocol_v1_crc16_ccitt_false(temp, (uint32_t)in_len);
        if (computed_crc != hdr->crc16) {
            return pack_rsp(hdr->seq, 0, PROTO_ERR_CRC, NULL, 0, out_packet, out_capacity);
        }
    }

    cmd = (const protocol_v1_cmd_envelope_t *)(in_packet + sizeof(protocol_v1_header_t));
    cmd_payload = cmd->payload;
    cmd_payload_len = (uint16_t)(hdr->len - sizeof(protocol_v1_cmd_envelope_t));

    switch (cmd->cmd_id) {
    case PROTO_CMD_GET_CAPS:
        return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, &k_device_caps, sizeof(k_device_caps), out_packet, out_capacity);

    case PROTO_CMD_GET_SCOPE_CFG:
        return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, &state->scope_cfg, sizeof(state->scope_cfg), out_packet, out_capacity);

    case PROTO_CMD_SET_SCOPE_CFG:
        if (cmd_payload_len != sizeof(protocol_v1_scope_cfg_t)) {
            return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_ERR_BAD_ARG, NULL, 0, out_packet, out_capacity);
        }
        {
            const protocol_v1_scope_cfg_t *cfg = (const protocol_v1_scope_cfg_t *)cmd_payload;
            if (!valid_scope_cfg(cfg)) {
                return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_ERR_BAD_ARG, NULL, 0, out_packet, out_capacity);
            }
            state->scope_cfg = *cfg;
            return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, &state->scope_cfg, sizeof(state->scope_cfg), out_packet, out_capacity);
        }

    case PROTO_CMD_GET_FG_CFG:
        return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, &state->fg_cfg, sizeof(state->fg_cfg), out_packet, out_capacity);

    case PROTO_CMD_SET_FG_CFG:
        if (cmd_payload_len != sizeof(protocol_v1_fg_cfg_t)) {
            return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_ERR_BAD_ARG, NULL, 0, out_packet, out_capacity);
        }
        {
            const protocol_v1_fg_cfg_t *cfg = (const protocol_v1_fg_cfg_t *)cmd_payload;
            if (!valid_fg_cfg(cfg)) {
                return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_ERR_BAD_ARG, NULL, 0, out_packet, out_capacity);
            }
            state->fg_cfg = *cfg;
            return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, &state->fg_cfg, sizeof(state->fg_cfg), out_packet, out_capacity);
        }

    case PROTO_CMD_STREAM_START:
        state->streaming = true;
        return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, NULL, 0, out_packet, out_capacity);

    case PROTO_CMD_STREAM_STOP:
        state->streaming = false;
        return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, NULL, 0, out_packet, out_capacity);

    case PROTO_CMD_PING:
        return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_STATUS_OK, cmd_payload, cmd_payload_len, out_packet, out_capacity);

    default:
        return pack_rsp(hdr->seq, cmd->cmd_id, PROTO_ERR_BAD_CMD, NULL, 0, out_packet, out_capacity);
    }
}
