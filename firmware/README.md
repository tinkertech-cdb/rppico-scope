# Firmware Build Notes

This firmware scaffold targets Pico SDK 2.2.0+.

## Prerequisites

- Pico SDK installed (example: `~/pico-sdk`)
- `cmake`, `ninja` (or make), and ARM GCC toolchain

## Configure and Build

```bash
export PICO_SDK_PATH="$HOME/pico-sdk"
cmake -S firmware -B firmware/build -G Ninja -DPICO_BOARD=pico2
cmake --build firmware/build
```

For a dedicated Seeed XIAO RP2350 board definition in your SDK, replace `-DPICO_BOARD=pico2` with the matching board name.

Optional USB identity overrides:

```bash
cmake -S firmware -B firmware/build -G Ninja \
  -DPICO_BOARD=pico2 \
  -DRPPICO_USB_VID=0xCAFE \
  -DRPPICO_USB_PID=0x4010 \
  -DRPPICO_USB_MANUFACTURER="OpenScope Community" \
  -DRPPICO_USB_PRODUCT="XIAO RP2350 Scope+FG"
```

## Current Firmware Scope

- Shared protocol V1 types are consumed from `protocol/include/protocol_v1.h`.
- `protocol_engine` currently handles core command dispatch and config validation.
- USB vendor bulk endpoint wiring is pending; bootstrap currently validates protocol/CRC and engine responses in firmware runtime.
