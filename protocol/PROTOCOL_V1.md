# Protocol V1

This document defines the wire protocol between the Linux Qt viewer and RP2350 firmware.

## 1. Scope

- Transport for protocol traffic: USB vendor bulk only
- CDC usage: logs and diagnostics only (not protocol state)
- Endianness: little-endian for all multi-byte fields

## 2. Packet Model

All traffic uses a common packet header plus payload.

### 2.1 Header

```c
typedef struct __attribute__((packed)) {
    uint16_t magic;    // 0x4453 ('DS')
    uint8_t  version;  // 1
    uint8_t  kind;     // PacketKind
    uint16_t seq;      // host-driven sequence for CMD/RSP, device sequence for STREAM/EVENT
    uint16_t len;      // payload bytes
    uint16_t crc16;    // CRC-16 over header (without crc16 field) + payload
} PacketHeader;
```

`PacketKind`:

- `1`: `CMD`
- `2`: `RSP`
- `3`: `STREAM`
- `4`: `EVENT`

### 2.2 CRC

- Algorithm: CRC-16/CCITT-FALSE
- Polynomial: `0x1021`
- Init: `0xFFFF`
- RefIn/RefOut: false
- XorOut: `0x0000`

## 3. Command/Response

### 3.1 Command Envelope (`CMD`)

```c
typedef struct __attribute__((packed)) {
    uint16_t cmd_id;
    uint16_t reserved;
    uint8_t  payload[];
} CmdEnvelope;
```

### 3.2 Response Envelope (`RSP`)

```c
typedef struct __attribute__((packed)) {
    uint16_t cmd_id;   // echoed command id
    uint16_t status;   // StatusCode
    uint8_t  payload[];
} RspEnvelope;
```

`StatusCode`:

- `0`: `OK`
- `1`: `ERR_BAD_CMD`
- `2`: `ERR_BAD_ARG`
- `3`: `ERR_STATE`
- `4`: `ERR_BUSY`
- `5`: `ERR_UNSUPPORTED`
- `6`: `ERR_CRC`
- `7`: `ERR_INTERNAL`

### 3.3 Command IDs

- `0x0001`: `GET_CAPS`
- `0x0002`: `SET_SCOPE_CFG`
- `0x0003`: `GET_SCOPE_CFG`
- `0x0004`: `STREAM_START`
- `0x0005`: `STREAM_STOP`
- `0x0006`: `SET_FG_CFG`
- `0x0007`: `GET_FG_CFG`
- `0x0008`: `PING`

## 4. Data Structures

### 4.1 ScopeConfig

```c
typedef struct __attribute__((packed)) {
    uint8_t  channel_mask;      // bit0 CH1, bit1 CH2
    uint8_t  run_mode;          // 0 Auto, 1 Normal, 2 Single
    uint8_t  trigger_source;    // 0 CH1, 1 CH2
    uint8_t  trigger_edge;      // 0 Rising, 1 Falling
    uint8_t  trigger_level;     // 8-bit ADC domain
    uint8_t  pretrigger_pct;    // 0..90
    uint16_t reserved0;
    uint32_t sample_rate_hz;    // per-channel request, firmware clamps to safe limits
    uint32_t timebase_ps;       // optional hint for UI-driven autoset
} ScopeConfig;
```

### 4.2 FgConfig

```c
typedef struct __attribute__((packed)) {
    uint8_t  waveform;          // 0 Sine, 1 Square, 2 Triangle, 3 Saw, 4 DC
    uint8_t  enabled;           // 0 Off, 1 On
    uint16_t reserved0;
    uint32_t frequency_millihz;
    int16_t  amplitude_mvpp;
    int16_t  offset_mv;
} FgConfig;
```

### 4.3 Capabilities

`GET_CAPS` response payload:

```c
typedef struct __attribute__((packed)) {
    uint32_t feature_bits;      // bitset for optional features
    uint16_t max_channels;      // 2
    uint16_t max_adc_ksps_agg;  // 500
    uint32_t min_sample_rate_hz;
    uint32_t max_sample_rate_hz;
    uint32_t max_stream_payload;
    uint32_t fg_freq_min_millihz;
    uint32_t fg_freq_max_millihz;
    int16_t  fg_amp_min_mvpp;
    int16_t  fg_amp_max_mvpp;
    int16_t  fg_offset_min_mv;
    int16_t  fg_offset_max_mv;
} DeviceCaps;
```

`feature_bits` (initial):

- bit0: dual channel scope
- bit1: function generator
- bit2: trigger single mode
- bit3: stream drop counter

## 5. Streaming (`STREAM`)

Single frame format for all scope modes.

```c
typedef struct __attribute__((packed)) {
    uint32_t frame_id;
    uint32_t sample_rate_hz;    // effective per-channel rate
    uint8_t  channel_mask;      // bit0 CH1, bit1 CH2
    uint8_t  flags;             // bit0 triggered, bit1 single, bit2 overflow, bit3 dropped_prior
    int32_t  trigger_index;     // interleaved sample index, -1 if none
    uint32_t timestamp_ticks;
    uint16_t sample_count;      // interleaved points
    uint8_t  samples[];         // interleaved bytes: CH1,CH2,CH1,CH2...
} StreamFrame;
```

Notes:

- `sample_count` counts interleaved points, not per-channel points.
- For 1CH, samples are contiguous 8-bit values for that channel.

## 6. Events (`EVENT`)

```c
typedef struct __attribute__((packed)) {
    uint16_t event_id;
    uint8_t  severity;          // 0 info, 1 warn, 2 error
    uint8_t  reserved;
    uint8_t  payload[];
} EventPacket;
```

Event IDs:

- `0x0001`: `ERROR`
- `0x0002`: `WARN_DROPPED_FRAMES`
- `0x0003`: `USB_RESTARTED`
- `0x0004`: `FG_FORCED_OFF`

## 7. State Rules

- `SET_SCOPE_CFG` and `SET_FG_CFG` are valid in idle and streaming states.
- `STREAM_START` is rejected with `ERR_STATE` when already streaming.
- `STREAM_STOP` is idempotent and returns `OK` when already stopped.
- On USB disconnect or protocol fatal error, firmware forces FG off.

## 8. Versioning Rules

- `version` in header is mandatory and currently `1`.
- Unknown `cmd_id` must return `ERR_BAD_CMD`.
- Unsupported but valid command for this device returns `ERR_UNSUPPORTED`.

## 9. MVP Limits

- Guaranteed ADC limit: `500 kS/s` aggregate.
- Typical per-channel max in dual-channel mode: `~250 kS/s` each.
- Host must treat higher rates as unsupported unless explicitly advertised by `DeviceCaps`.
