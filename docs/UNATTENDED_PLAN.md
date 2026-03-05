# Unattended Project Completion Plan

## Scope Lock (MVP)

- Device: Seeed XIAO RP2350
- Board strategy: manufacturer-independent architecture; initial MVP board is Seeed XIAO RP2350
- Firmware: C (Pico SDK 2.2.0+)
- Host: Linux Qt 6 (C++)
- License: AGPL-3.0-or-later
- Features:
  - 2-channel oscilloscope
  - 1-channel function generator
  - Host-side FFT + THD+N
- USB transport:
  - Vendor bulk for full protocol (CMD/RSP/STREAM/EVENT)
  - CDC for logs/diagnostics only
- Pinout (MVP):
  - Scope CH1 on `ADC0`
  - Scope CH2 on `ADC1`
  - FG PWM output on `ADC2 pin` (GPIO mode)
- Analog frontend (MVP):
  - Divider + `1.65V` offset
  - Input envelope target: `-1.6V..+1.6V` (`3.2Vpp`)
- FG generation method: PWM
- USB identity (development default):
  - VID `0xCAFE`, PID `0x4010`
  - Configurable at build time
- Safe ADC envelope:
  - 500 kS/s aggregate guaranteed
  - ~250 kS/s/ch in 2CH mode

## Delivery Strategy

Build in vertical slices so each milestone is runnable:
1. Connect + configure
2. Stream + render
3. Trigger stabilization
4. Measurements
5. FFT + THD+N
6. Function generator
7. Hardening + release docs

Portability constraint for all milestones:

- Board-specific details (pin maps, clocks, peripheral setup) must be isolated so additional RP2350 boards can be added without protocol or UI redesign.

## Milestones and Acceptance Criteria

### M1 - Foundations
Tasks:
- Finalize protocol contract (`protocol/PROTOCOL_V1.md`)
- Add shared protocol headers (`protocol/include/protocol_v1.h`)
- Scaffold `viewer/` and top-level build/readme docs
- Add coding/format/test conventions

Acceptance:
- Firmware and viewer compile with shared protocol constants/types
- Minimal protocol encode/decode unit tests pass

### M2 - Firmware Transport + Control
Tasks:
- Implement USB composite behavior:
  - Vendor bulk protocol pipe (all protocol traffic)
  - CDC logging only
- Implement packet parser (header, CRC16, seq handling)
- Implement command dispatcher:
  - GET_CAPS, GET/SET_SCOPE_CFG, STREAM_START/STOP, PING
- Implement error/status event emission

Acceptance:
- Host can connect, ping, read caps, configure scope, start/stop stream commands successfully

### M3 - Acquisition + Streaming
Tasks:
- Implement ADC + DMA capture path (1CH/2CH)
- Implement unified acquisition state machine (Auto/Normal/Single flags)
- Implement trigger metadata generation
- Emit unified STREAM frames with required metadata fields

Acceptance:
- Stable stream at safe rates
- 1CH and 2CH data ordering validated
- Trigger metadata present and consistent

### M4 - Qt Viewer Core
Tasks:
- Implement USB transport backend for vendor bulk protocol
- Implement protocol parser and reconnect logic
- Build live waveform view for CH1/CH2
- Add controls:
  - channels, run mode, trigger source/edge, sample-rate/timebase presets
- Surface errors/events in status UI

Acceptance:
- User can control firmware from UI and see live traces with reconnect resilience

### M5 - Measurements + FFT + THD+N
Tasks:
- Implement time-domain metrics:
  - min, max, mean, Vpp, Vrms, frequency
- Implement FFT pipeline with selectable windows (at least Hann)
- Implement THD+N computation path and panel
- Throttle analysis updates independently of render cadence

Acceptance:
- Metrics and FFT/THD+N produce stable, repeatable results on known test tones

### M6 - Function Generator Integration
Tasks:
- Implement GET/SET_FG_CFG firmware path
- Add FG UI controls:
  - waveform, freq, amplitude, offset, enable
- Implement safe-off behavior on disconnect/reset/fatal errors
- Emit FG safety events (`FG_FORCED_OFF`)

Acceptance:
- FG parameters round-trip correctly and output safety behavior is verified

### M7 - Hardening + Release
Tasks:
- Stress and soak tests for stream reliability
- Malformed packet and CRC error recovery tests
- Dropped-frame handling and telemetry validation
- Docs completion:
  - build/run guides, architecture, troubleshooting, contribution guide
- AGPL/SPDX/license file consistency pass

Acceptance:
- Project is reproducible, documented, and ready for public Codeberg release

## CI and Automation Plan

- Firmware CI:
  - Configure/build for `pico2`
  - Upload artifacts (`.elf`, `.uf2`)
- Viewer CI (Linux):
  - Build + unit tests
- Protocol CI:
  - Serialization/CRC fixture tests
- Nightly integration:
  - Replay captured stream fixtures through viewer parser/analysis
- Initial validation fixture strategy:
  - Use loopback/self-test patterns until external reference gear is available

## USB Identity and Release Policy

- Keep development VID/PID (`0xCAFE:0x4010`) for local/testing builds.
- Use unique-ID-derived USB serial for deterministic multi-device mapping.
- Keep VID/PID and USB strings configurable via build options.
- Before broad public release, migrate to a legally appropriate production VID/PID.

## Hosting Policy

- Primary forge: Codeberg only.
- Canonical repository: `codeberg.org/tinkertech-cdb/rppico-scope`.
- No mirror to other forges during MVP unless explicitly enabled later.

## Risk Register and Mitigations

- Throughput bottlenecks:
  - Keep bounded STREAM payload sizes and frame queue limits
- Trigger instability:
  - Start with robust metadata-first approach; optimize after correctness
- USB protocol fragility:
  - Strict parser + deterministic error codes + reconnect state machine
- Scope creep:
  - Defer non-goals to post-MVP backlog and keep protocol versioned

## Operational Rules for Unattended Execution

- Small, milestone-tagged PR-sized increments
- Branch strategy: `main` protected + short-lived feature branches
- Commit style: Conventional Commits
- Each increment includes:
  - implementation
  - test/update
  - docs delta
- No protocol-breaking change without version bump and migration note
- Preserve MVP safety constraints in UI and firmware limits
- Publish milestone tags (`M1..M7`) and changelog entries per milestone

## Post-MVP Backlog (Deferred)

- External ADC migration path
- Windows/macOS viewer support
- Advanced trigger/protocol decode features
- Calibration workflow and tooling
