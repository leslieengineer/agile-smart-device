#include <board/Board.hpp>
#include <services/BinarySwitchService.hpp>
#include <smart_device/SmartDevice.hpp>
#include <smart_device/SmartDeviceApplication.hpp>

#include "SwitchRuntime.hpp"
#include "NvsBinaryStateStore.hpp"
#include "nvs_flash.h"

namespace smart_device {
namespace {

uhal::Status initialize_nvs() {
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (nvs_flash_erase() != ESP_OK) return uhal::Status::io_error;
        error = nvs_flash_init();
    }
    return error == ESP_OK ? uhal::Status::ok : uhal::Status::io_error;
}

}  // namespace

uhal::Status start() {
    if (initialize_nvs() != uhal::Status::ok) return uhal::Status::io_error;

    static board::Board board;
    if (board.initialize() != uhal::Status::ok) return uhal::Status::io_error;

    static NvsBinaryStateStore           store;
    static services::BinarySwitchService service{board.relay(), board.led(), store};
    static SmartDeviceApplication        application{service};
    if (application.initialize() != uhal::Status::ok) return uhal::Status::io_error;

    static SwitchRuntime runtime{board, application};
    return runtime.start();
}

}  // namespace smart_device
