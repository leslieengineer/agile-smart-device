# agile-smart-device

ESP-IDF application for the ESP32-C6.

## Prerequisites

- ESP-IDF v6.x
- ESP32-C6 toolchain installed by ESP-IDF
- Initialized `external/agile-firmware-framework` submodule

## Clone

```cmd
git clone --recurse-submodules git@github.com:leslieengineer/agile-smart-device.git
git submodule update --init --recursive
```

## Architecture

- `main` provides `app_main()` and platform logging.
- `components/product_smart_device` owns the Layer 5 composition root.
- `components/framework_uhal_core` bridges the product to framework UHAL core.
- `external/agile-firmware-framework` contains reusable capabilities, not product code.

Additional framework capabilities must be imported through dedicated bridge components rather than direct include paths in product code. Product-specific board configuration and services stay in this repository.

## Build on Windows

```cmd
call C:\Users\lesli\espv6\v6.0.2\esp-idf\export.bat
cd /d C:\Users\lesli\WS\agile-smart-device
idf.py set-target esp32c6
idf.py build
```

The current milestone only verifies compilation. It does not flash or monitor a board.
