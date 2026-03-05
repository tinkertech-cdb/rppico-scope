# Implementation Status

## Completed

- Shared protocol header added: `protocol/include/protocol_v1.h`
- Firmware CRC implementation added: `firmware/src/protocol_v1_crc.c`
- Firmware protocol engine added with command handling:
  - `GET_CAPS`
  - `GET_SCOPE_CFG`, `SET_SCOPE_CFG`
  - `GET_FG_CFG`, `SET_FG_CFG`
  - `STREAM_START`, `STREAM_STOP`
  - `PING`
- Firmware defaults and limits bound to locked MVP assumptions:
  - CH1 on `ADC0`, CH2 on `ADC1`, FG PWM on `ADC2` pin (GPIO mode)
  - 500 kS/s aggregate safe ADC limit
- Firmware USB identity made build-configurable via CMake cache vars
- Qt 6 viewer scaffold extended with session controls:
  - Connect/Disconnect
  - Ping
  - Get Caps
  - Stream Start/Stop
- Qt protocol codec added for packet build/parse + CRC verification

## Current Transport Mode

- Viewer transport is currently mock-backed to validate protocol/UI flow.
- Firmware protocol engine is integrated and callable in firmware runtime.
- USB vendor bulk endpoint wiring is next.

## Next Priority

1. Add TinyUSB descriptors for CDC + vendor bulk interfaces in firmware.
2. Wire vendor OUT packet RX to protocol engine and vendor IN for responses/events.
3. Replace viewer mock session with real bulk transport backend.
