# Functional Specification Document (FSD)

## Project

USB DSO + Function Generator firmware and Linux Qt control/viewer for RP2350-class boards (initial target: Seeed XIAO RP2350).

## 1. Purpose

Build an open-source instrument platform composed of:

- RP2350 firmware (Pico SDK, C) running on Seeed XIAO RP2350
- Linux desktop Qt application (C++) for control and visualization
- Shared host/device binary protocol for control and streaming

The platform provides:

- 2-channel oscilloscope
- 1-channel function generator
- FFT and THD+N analysis in the host application

## 2. Locked Decisions

- License: `AGPL-3.0-or-later`
- Host target (MVP): Linux
- Main languages: C (firmware), C++ (Qt 6 viewer)
- USB transport: Hybrid
  - Vendor bulk endpoints for waveform streaming and binary protocol control
  - CDC for logs/diagnostics only
- ADC strategy (MVP): On-chip ADC with PIO-assisted timing/control architecture
- Pinout (MVP): `ADC0` = Scope CH1, `ADC1` = Scope CH2, `ADC2 pin` = FG PWM output
- Analog frontend (MVP): input divider + `1.65V` DC offset, expected input envelope `-1.6V..+1.6V` (`3.2Vpp`)
- Function generator output method (MVP): PWM
- Safe sampling limit: `500 kS/s aggregate` (guaranteed)
  - 1CH mode: up to 500 kS/s
  - 2CH mode: up to ~250 kS/s per channel (aggregate stays 500 kS/s)
- USB identity (dev default): VID `0xCAFE`, PID `0x4010`, configurable for release migration

## 3. Goals

- Reliable USB waveform streaming and robust reconnect behavior
- Usable two-channel scope UX with common controls
- Host-side FFT and THD+N analysis suitable for hobbyist/educational use
- Remote function generator control from Qt app
- Clean architecture that can later support external ADC hardware
- Board/manufacturer-independent architecture beyond MVP board support
- Public Codeberg readiness with clear docs and licensing

## 4. Non-Goals (MVP)

- Lab-grade calibration and certified metrology
- Guaranteed operation above safe ADC envelope (no >500 kS/s guarantees)
- Protocol decoder suite (I2C/SPI/UART) in initial release
- Deep segmented memory capture in initial release
- Full calibration workflow UI beyond basic offset/gain controls

## 5. System Architecture

Portability principle:

- Keep board-specific pin/peripheral mapping in dedicated configuration modules.
- Keep protocol, acquisition state machine, and host app independent of board vendor.
- MVP board is Seeed XIAO RP2350; future board profiles are first-class extension targets.

### 5.1 Firmware (RP2350)

Core modules:

- `acq_core`: unified acquisition state machine (all run modes via flags)
- `acq_adc`: ADC setup and channel selection
- `acq_pio`: timing/trigger-assist setup
- `acq_dma`: DMA ring/buffer and ISR service
- `usb_link`: CDC logs + vendor bulk protocol transport
- `proto`: compact V1 packet encode/decode (`CMD/RSP/STREAM/EVENT`)
- `fg_core`: function generator control/state
- `device_state`: capabilities, limits, config, health

### 5.2 Desktop Viewer (Qt, Linux)

Core modules:

- `transport`: vendor bulk protocol backend (optional CDC log console monitor)
- `protocol`: frame parser/serializer and capability negotiation
- `capture_model`: waveform frame queue and metadata
- `render`: time-domain trace rendering for CH1/CH2
- `analysis`: measurements, FFT, THD+N
- `fg_panel`: waveform/frequency/amplitude/offset controls
- `settings`: app/session persistence

### 5.3 Data Flow

1. Host connects and requests capabilities.
2. Host applies scope/FG configuration.
3. Firmware configures ADC/PIO/DMA pipeline.
4. Firmware streams framed data over vendor bulk IN.
5. Host parses frames, renders traces, computes analysis.
6. Control, responses, stream frames, and events flow over vendor bulk; CDC is reserved for logs/diagnostics.

## 6. Functional Requirements

### 6.1 Oscilloscope

- FR-DSO-001: Support CH1 and CH2 enable/disable.
- FR-DSO-002: Support run modes: Auto, Normal, Single.
- FR-DSO-003: Support trigger source selection CH1/CH2.
- FR-DSO-004: Support trigger edge polarity (rising/falling).
- FR-DSO-005: Support timebase and sample-rate presets within safe limits.
- FR-DSO-006: Include frame metadata (seq, channel mask, sample rate, trigger info, timestamp ticks).
- FR-DSO-007: Continuous streaming mode with dropped-frame indication.
- FR-DSO-008: Pre/post-trigger buffering for stable trigger-centered display.

### 6.2 Measurements and Analysis (Host)

- FR-ANA-001: Basic time-domain measurements per selected channel: min, max, mean, Vpp, Vrms, frequency.
- FR-ANA-002: FFT view with selectable window function (at least Hann).
- FR-ANA-003: THD+N computation using selected channel and analysis settings.
- FR-ANA-004: Analysis update cadence decoupled from rendering cadence.

### 6.3 Function Generator

- FR-FG-001: 1-channel output control from host.
- FR-FG-002: Output waveforms: sine, square, triangle, sawtooth, DC.
- FR-FG-003: Frequency control within firmware-advertised limits.
- FR-FG-004: Amplitude control within hardware-safe limits.
- FR-FG-005: Offset control if hardware path supports it (otherwise capability-disabled).
- FR-FG-006: Explicit output enable/disable and safe-off on disconnect/reset.

### 6.4 Protocol (Simplified V1)

- FR-PROTO-001: Use a single versioned binary protocol over vendor bulk endpoints.
- FR-PROTO-002: Protocol packet kinds are only `CMD`, `RSP`, `STREAM`, `EVENT`.
- FR-PROTO-003: Use fixed little-endian packed structs in V1 (no TLV/JSON in MVP).
- FR-PROTO-004: Use sequence number + CRC for integrity and troubleshooting.
- FR-PROTO-005: Expose capabilities via `GET_CAPS` as bitmasks and limits table.
- FR-PROTO-006: Scope and FG configuration use single struct commands (`ScopeConfig`, `FgConfig`).
- FR-PROTO-007: Streaming uses one unified `STREAM` frame format for all run modes.
- FR-PROTO-008: Deterministic async events via `EVENT` for errors and stream health.

### 6.5 Reliability and UX

- FR-REL-001: Firmware must tolerate malformed packets without reboot.
- FR-REL-002: Host auto-reconnect workflow after unplug/replug.
- FR-REL-003: UI clearly surfaces transport and dropped-frame conditions.
- FR-REL-004: FG output transitions to safe state on app exit/disconnect.

## 7. Performance Envelope (MVP)

Guaranteed operating envelope:

- ADC aggregate sampling rate: up to `500 kS/s`
- Single-channel scope: up to `500 kS/s`
- Two-channel scope: up to `~250 kS/s` per channel

Notes:

- Any higher rates are outside MVP guarantees and must not be presented as nominal behavior.
- If future experimental overclock modes are added, UI/protocol must mark them as experimental.

## 8. USB Transport Design

### 8.1 Hybrid Transport Rationale

- Keep hybrid USB, but simplify ownership:
  - Vendor bulk carries the full protocol (control + stream + events).
  - CDC is debug/log console only and not part of app protocol state.
- This preserves debug convenience while removing dual-path protocol complexity.

### 8.2 Transport Split (Simplified)

- CDC:
  - Human-readable logs, diagnostics, and optional developer console.
- Vendor bulk OUT:
  - Host-to-device `CMD` packets.
- Vendor bulk IN:
  - Device-to-host `RSP`, `STREAM`, and `EVENT` packets.

### 8.3 USB Identity Policy

- Development default identity:
  - VID: `0xCAFE`
  - PID: `0x4010`
  - Manufacturer string: `OpenScope Community`
  - Product string: `XIAO RP2350 Scope+FG`
- Serial number must be derived from RP2350 unique ID for stable multi-device host mapping.
- VID/PID and strings must be compile-time configurable (CMake/cache options).
- Before broad public release, migrate to a legally appropriate production VID/PID allocation.

### 8.4 Protocol V1 Wire Format

Header (all packets, little-endian):

- `magic` (`u16`)
- `version` (`u8`)
- `kind` (`u8`) where:
  - `1=CMD`, `2=RSP`, `3=STREAM`, `4=EVENT`
- `seq` (`u16`)
- `len` (`u16`) payload length in bytes
- `crc16` (`u16`) over header (excluding crc field) + payload

#### 8.4.1 Command Envelope (`CMD`)

Payload:

- `cmd_id` (`u16`)
- `reserved` (`u16`)
- `cmd_payload` (variable)

Command IDs:

- `GET_CAPS`
- `SET_SCOPE_CFG`
- `GET_SCOPE_CFG`
- `STREAM_START`
- `STREAM_STOP`
- `SET_FG_CFG`
- `GET_FG_CFG`
- `PING`

#### 8.4.2 Response Envelope (`RSP`)

Payload:

- `cmd_id` (`u16`) command being answered
- `status` (`u16`) (`0=OK`, nonzero=error code)
- `rsp_payload` (variable, optional)

#### 8.4.3 Stream Frame (`STREAM`)

Payload:

- `frame_id` (`u32`)
- `sample_rate_hz` (`u32`) per-channel effective rate
- `channel_mask` (`u8`) bit0=CH1 bit1=CH2
- `flags` (`u8`) bitfield:
  - bit0: triggered
  - bit1: single-shot frame
  - bit2: overflow occurred
  - bit3: dropped prior frame(s)
- `trigger_index` (`i32`) sample index in interleaved frame, `-1` if none
- `timestamp_ticks` (`u32`)
- `sample_count` (`u16`) number of interleaved sample points
- `samples` (`u8[]`) interleaved ADC bytes (CH1,CH2,...)

#### 8.4.4 Event Packet (`EVENT`)

Payload:

- `event_id` (`u16`)
- `severity` (`u8`) (`info/warn/error`)
- `reserved` (`u8`)
- `event_data` (variable)

Event IDs:

- `ERROR`
- `WARN_DROPPED_FRAMES`
- `USB_RESTARTED`
- `FG_FORCED_OFF`

## 9. Firmware Design Details

### 9.1 Acquisition and Triggering

- Use one acquisition pipeline with mode flags (Auto/Normal/Single), rather than separate mode-specific engines.
- ADC configured for 1CH/2CH with shared DMA path.
- Single DMA ISR/service flow for both channel configurations.
- Trigger metadata attached to each `STREAM` frame.

### 9.2 PIO Usage in MVP

- PIO used for timing/trigger-assist infrastructure and deterministic control edges.
- ADC conversion remains via on-chip ADC path.
- Architecture keeps abstraction boundary to support future external ADC + direct PIO sampling.

### 9.3 Safety and Fault Handling

- Recoverable protocol parser errors.
- Timeout and buffer-overrun telemetry counters.
- Fail-safe FG disable path from USB disconnect, watchdog, or fatal acquisition error.

## 10. Qt Viewer Design Details

### 10.1 Scope UI

- Live dual-trace rendering.
- Timebase, vertical scale, trigger controls.
- Run/Stop/Single controls.

### 10.2 Analysis UI

- Measurement table and per-channel readouts.
- FFT panel with window selection and dominant peak display.
- THD+N panel with channel and analysis bandwidth selection.

### 10.3 FG UI

- Waveform selector.
- Frequency/amplitude/offset controls.
- Output enable toggle with explicit state feedback.

## 11. Verification Plan (MVP)

- V-001: USB connect/reconnect stress test.
- V-002: Streaming integrity under sustained capture at safe max rates.
- V-003: Trigger repeatability with known periodic input.
- V-004: Frequency estimate and FFT peak placement sanity tests.
- V-005: THD+N sanity checks with synthetic tone sets.
- V-006: FG control sweep tests with safe shutdown validation.
- V-007: Long-run stability test (memory, lockups, transport errors).
- V-008: Initial validation fixtures use loopback/self-test patterns until external reference instruments are added.

## 12. Repository Layout

- `firmware/` : RP2350 firmware sources and CMake.
- `viewer/` : Qt Linux application sources and build files.
- `protocol/` : protocol specification and shared constants docs (`protocol/PROTOCOL_V1.md`).
- `docs/` : architecture, setup, validation notes.
- `hardware/` : optional pinout/analog frontend notes.

## 13. Milestones

- M1: Repo skeleton, protocol draft, USB command loop.
- M2: 1CH capture + host live plot.
- M3: 2CH capture + trigger controls.
- M4: Function generator control path.
- M5: FFT + THD+N + core measurements.
- M6: Stability pass, docs, release packaging.

## 14. Licensing and Community

- Top-level license: `AGPL-3.0-or-later`.
- Add SPDX headers in source files.
- Include `CONTRIBUTING.md` and coding/build guidelines.
- Publish issue templates and roadmap for community contribution.

## 15. Hosting Policy

- Primary forge: Codeberg only.
- Canonical repository: `codeberg.org/tinkertech-cdb/rppico-scope`.
- No mirror to other forges during MVP unless explicitly enabled later.

## 16. Open Items for Later Revisions

- External ADC migration path and compatibility mode.
- Calibration workflow and reference procedure.
- Windows/macOS host support.
- Protocol decoder and advanced math features.
