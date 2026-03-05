# Qt Viewer Build Notes

Linux host target uses Qt 6 Widgets.

## Prerequisites

- Qt 6 development packages
- `cmake` and `ninja`

## Configure and Build

```bash
cmake -S viewer -B viewer/build -G Ninja
cmake --build viewer/build
```

## Current Runtime Mode

- The UI currently uses a mock transport backend.
- Protocol packet build/parse and CRC paths are active in the app.
- Real USB vendor bulk transport is the next integration step.
