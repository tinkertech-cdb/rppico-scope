#ifndef PROTOCOL_V1_H
#define PROTOCOL_V1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_V1_MAGIC 0x4453u
#define PROTOCOL_V1_VERSION 1u

enum protocol_v1_packet_kind {
    PROTO_KIND_CMD = 1,
    PROTO_KIND_RSP = 2,
    PROTO_KIND_STREAM = 3,
    PROTO_KIND_EVENT = 4,
};

enum protocol_v1_status_code {
    PROTO_STATUS_OK = 0,
    PROTO_ERR_BAD_CMD = 1,
    PROTO_ERR_BAD_ARG = 2,
    PROTO_ERR_STATE = 3,
    PROTO_ERR_BUSY = 4,
    PROTO_ERR_UNSUPPORTED = 5,
    PROTO_ERR_CRC = 6,
    PROTO_ERR_INTERNAL = 7,
};

enum protocol_v1_cmd_id {
    PROTO_CMD_GET_CAPS = 0x0001,
    PROTO_CMD_SET_SCOPE_CFG = 0x0002,
    PROTO_CMD_GET_SCOPE_CFG = 0x0003,
    PROTO_CMD_STREAM_START = 0x0004,
    PROTO_CMD_STREAM_STOP = 0x0005,
    PROTO_CMD_SET_FG_CFG = 0x0006,
    PROTO_CMD_GET_FG_CFG = 0x0007,
    PROTO_CMD_PING = 0x0008,
};

enum protocol_v1_fg_waveform {
    PROTO_FG_SINE = 0,
    PROTO_FG_SQUARE = 1,
    PROTO_FG_TRIANGLE = 2,
    PROTO_FG_SAW = 3,
    PROTO_FG_DC = 4,
};

enum protocol_v1_scope_run_mode {
    PROTO_SCOPE_AUTO = 0,
    PROTO_SCOPE_NORMAL = 1,
    PROTO_SCOPE_SINGLE = 2,
};

enum protocol_v1_trigger_source {
    PROTO_TRIGGER_CH1 = 0,
    PROTO_TRIGGER_CH2 = 1,
};

enum protocol_v1_trigger_edge {
    PROTO_TRIGGER_RISING = 0,
    PROTO_TRIGGER_FALLING = 1,
};

enum protocol_v1_event_id {
    PROTO_EVENT_ERROR = 0x0001,
    PROTO_EVENT_WARN_DROPPED_FRAMES = 0x0002,
    PROTO_EVENT_USB_RESTARTED = 0x0003,
    PROTO_EVENT_FG_FORCED_OFF = 0x0004,
};

enum protocol_v1_event_severity {
    PROTO_SEV_INFO = 0,
    PROTO_SEV_WARN = 1,
    PROTO_SEV_ERROR = 2,
};

enum protocol_v1_feature_bits {
    PROTO_FEAT_DUAL_CHANNEL_SCOPE = 1u << 0,
    PROTO_FEAT_FUNCTION_GENERATOR = 1u << 1,
    PROTO_FEAT_TRIGGER_SINGLE = 1u << 2,
    PROTO_FEAT_STREAM_DROP_COUNTER = 1u << 3,
};

#if defined(__GNUC__)
#define PROTO_PACKED __attribute__((packed))
#else
#define PROTO_PACKED
#endif

typedef struct PROTO_PACKED {
    uint16_t magic;
    uint8_t version;
    uint8_t kind;
    uint16_t seq;
    uint16_t len;
    uint16_t crc16;
} protocol_v1_header_t;

typedef struct PROTO_PACKED {
    uint16_t cmd_id;
    uint16_t reserved;
    uint8_t payload[];
} protocol_v1_cmd_envelope_t;

typedef struct PROTO_PACKED {
    uint16_t cmd_id;
    uint16_t status;
    uint8_t payload[];
} protocol_v1_rsp_envelope_t;

typedef struct PROTO_PACKED {
    uint8_t channel_mask;
    uint8_t run_mode;
    uint8_t trigger_source;
    uint8_t trigger_edge;
    uint8_t trigger_level;
    uint8_t pretrigger_pct;
    uint16_t reserved0;
    uint32_t sample_rate_hz;
    uint32_t timebase_ps;
} protocol_v1_scope_cfg_t;

typedef struct PROTO_PACKED {
    uint8_t waveform;
    uint8_t enabled;
    uint16_t reserved0;
    uint32_t frequency_millihz;
    int16_t amplitude_mvpp;
    int16_t offset_mv;
} protocol_v1_fg_cfg_t;

typedef struct PROTO_PACKED {
    uint32_t feature_bits;
    uint16_t max_channels;
    uint16_t max_adc_ksps_agg;
    uint32_t min_sample_rate_hz;
    uint32_t max_sample_rate_hz;
    uint32_t max_stream_payload;
    uint32_t fg_freq_min_millihz;
    uint32_t fg_freq_max_millihz;
    int16_t fg_amp_min_mvpp;
    int16_t fg_amp_max_mvpp;
    int16_t fg_offset_min_mv;
    int16_t fg_offset_max_mv;
} protocol_v1_device_caps_t;

typedef struct PROTO_PACKED {
    uint32_t frame_id;
    uint32_t sample_rate_hz;
    uint8_t channel_mask;
    uint8_t flags;
    int32_t trigger_index;
    uint32_t timestamp_ticks;
    uint16_t sample_count;
    uint8_t samples[];
} protocol_v1_stream_frame_t;

typedef struct PROTO_PACKED {
    uint16_t event_id;
    uint8_t severity;
    uint8_t reserved;
    uint8_t payload[];
} protocol_v1_event_t;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(protocol_v1_header_t) == 10, "protocol header size mismatch");
_Static_assert(sizeof(protocol_v1_scope_cfg_t) == 16, "scope config size mismatch");
_Static_assert(sizeof(protocol_v1_fg_cfg_t) == 12, "fg config size mismatch");
_Static_assert(sizeof(protocol_v1_device_caps_t) == 36, "device caps size mismatch");
_Static_assert(sizeof(protocol_v1_stream_frame_t) == 20, "stream frame header size mismatch");
#endif

uint16_t protocol_v1_crc16_ccitt_false(const uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
