# ESP32-C6 UHAL Adapters

This is architecture layer 3. Each adapter implements a UHAL contract through an ESP-IDF driver/API and converts `esp_err_t` to `uhal::Status`.

For example, `i2c/Esp32C6I2c` implements `uhal::II2c`; `can/Esp32C6Can` adapts ESP-IDF TWAI to the platform-neutral `uhal::ICan` contract.

Protocol and device rules do not belong here. This layer only translates the UHAL capability to ESP-IDF and holds configured hardware resources.
