#include <smart_device/SmartDevice.hpp>

#include "esp_log.h"

namespace {
constexpr char kTag[] = "agile-smart-device";
}

extern "C" void app_main(void) {
    const uhal::Status status = smart_device::start();
    if (status == uhal::Status::ok) {
        ESP_LOGI(kTag, "smart_device started");
    } else {
        ESP_LOGE(kTag, "smart_device start failed");
    }
}
