# INSTALL

Installation and build guide for `rppico-scope`.

## 1) Host Platform

MVP host target: Linux (Qt 6 desktop app)

## 2) Prerequisites

### Common

- `git`
- `cmake`
- `ninja` (recommended)
- C/C++ build tools

### Firmware toolchain

- ARM embedded GCC (`arm-none-eabi-gcc`)
- Pico SDK 2.2.0+
- `picotool` (optional but useful)

### Viewer toolchain

- Qt 6 development packages (Widgets)
- C++17 compiler

## 3) Clone

```bash
git clone git@codeberg.org:tinkertech-cdb/rppico-scope.git
cd rppico-scope
```

## 4) Build Firmware

Set Pico SDK path:

```bash
export PICO_SDK_PATH="$HOME/pico-sdk"
```

Configure and build:

```bash
cmake -S firmware -B firmware/build -G Ninja -DPICO_BOARD=pico2
cmake --build firmware/build
```

UF2 output:

- `firmware/build/src/rppico_scope_fw.uf2`

### Optional USB identity overrides

```bash
cmake -S firmware -B firmware/build -G Ninja \
  -DPICO_BOARD=pico2 \
  -DRPPICO_USB_VID=0xCAFE \
  -DRPPICO_USB_PID=0x4010 \
  -DRPPICO_USB_MANUFACTURER="OpenScope Community" \
  -DRPPICO_USB_PRODUCT="XIAO RP2350 Scope+FG"
cmake --build firmware/build
```

## 5) Flash Firmware (.uf2)

XIAO RP2350 supports UF2 drag-and-drop flashing:

1. Hold BOOT and connect board by USB
2. Board mounts as USB mass storage
3. Copy `firmware/build/src/rppico_scope_fw.uf2`
4. Board reboots into firmware

## 6) Build Viewer

```bash
cmake -S viewer -B viewer/build -G Ninja
cmake --build viewer/build
```

Run:

```bash
./viewer/build/rppico_scope_viewer
```

## 7) USB Access Notes (Linux)

Depending on distro, non-root USB access may require udev rules for your VID/PID.

If access fails:

- verify with `sudo` once to confirm permissions issue
- add a udev rule for your VID/PID and reconnect device

## 8) Troubleshooting

### Firmware configure fails (`PICO_SDK_PATH` missing)

```bash
echo "$PICO_SDK_PATH"
```

It should point to a valid Pico SDK path (example: `~/pico-sdk`).

### Qt configure fails

Install Qt 6 Widgets development components and rerun CMake.

### No data in viewer

Viewer transport may still be scaffold/mock depending on branch status. Check `docs/STATUS.md`.

## 9) Canonical Hosting

Codeberg is canonical:

- `https://codeberg.org/tinkertech-cdb/rppico-scope`

GitHub is optional mirror only.
