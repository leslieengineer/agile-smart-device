# agile-smart-device

ESP-IDF smart-switch firmware for nanoESP32-C6.

## Milestone 1

- One active-high relay on GPIO10.
- Active-low button with pull-up on GPIO9.
- Active-high status LED on GPIO2 mirrors the relay.
- Debounced short press toggles the relay.
- Relay state is restored from NVS after reboot.
- 16 MB flash, ESP-IDF v6.0.2.

## Architecture

`main` only calls the Layer 5 composition root in `product_smart_device`. Product policy and NVS schema live in this repository. Reusable button logic, UHAL contracts, and ESP32-C6 Layer 1/3 adapters live in the framework submodule and are imported through selective bridge components.

GPIO9 is a strapping pin; holding it during reset enters download mode. GPIO10 requires external fail-safe bias in the relay circuit so the relay remains off before firmware initialization.

## Build

```cmd
call C:\Users\lesli\espv6\v6.0.2\esp-idf\export.bat
git submodule update --init --recursive
idf.py set-target esp32c6
idf.py build
```
