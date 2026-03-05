# rppico-scope

Open-source USB instrument platform for RP2350-class boards (MVP target: Seeed XIAO RP2350), featuring:

- 2-channel oscilloscope (on-chip ADC, safe 500 kS/s aggregate)
- 1-channel PWM function generator
- Qt 6 Linux desktop viewer/control app
- Host-side FFT and THD+N analysis

License: AGPL-3.0-or-later

## Project Status

Early development. Architecture and protocol foundations are in place.

Current baseline includes:

- Shared Protocol V1 types (`protocol/include/protocol_v1.h`)
- Firmware command engine scaffold (`GET_CAPS`, config set/get, stream start/stop, `PING`)
- Qt 6 viewer scaffold with protocol packet codec and mock session flow
- Buildable firmware and viewer targets

See `docs/STATUS.md` for current progress.

## Scope and Design Principles

- Codeberg is the canonical repository host.
- Manufacturer-independent architecture is a core goal.
- Board-specific details are isolated from shared protocol and host app logic.
- MVP board profile is Seeed XIAO RP2350; design targets RP2350-class portability.

## Repository Layout

- `firmware/` - RP2350 firmware (Pico SDK, C)
- `viewer/` - Qt 6 Linux app (C++)
- `protocol/` - wire protocol spec and shared headers
- `docs/` - roadmap, hosting policy, unattended plan, status

## Protocol and Transport

Protocol reference: `protocol/PROTOCOL_V1.md`

Transport model:

- Vendor bulk endpoints carry protocol packets (`CMD`, `RSP`, `STREAM`, `EVENT`)
- CDC is reserved for logs/diagnostics

## Safety Envelope (MVP)

On-chip ADC guaranteed envelope:

- 500 kS/s aggregate max
- 1CH: up to 500 kS/s
- 2CH: up to ~250 kS/s per channel

Any higher rates are out of guaranteed scope unless explicitly introduced as experimental.

## Frontend Safety Notice

The analog frontend circuit guidance is for technically qualified users and requires electronics expertise before connecting to any RP2350 board, DUT, or external equipment. The project is provided as-is and accepts no liability for hardware damage, data loss, or injury from circuit implementation or use. See `FRONTEND_CIRCUIT.md`.

## Quick Start

See `INSTALL.md` for full setup.

Firmware build:

```bash
export PICO_SDK_PATH="$HOME/pico-sdk"
cmake -S firmware -B firmware/build -G Ninja -DPICO_BOARD=pico2
cmake --build firmware/build
```

Viewer build:

```bash
cmake -S viewer -B viewer/build -G Ninja
cmake --build viewer/build
```

## Development Workflow

Primary remote: Codeberg (`origin`)

Optional mirror remote: GitHub (`github`)

Typical flow:

1. `git push` (Codeberg canonical)
2. `git push-github-mirror` (updates `github/codeberg-main`)

See `docs/HOSTING.md`.
