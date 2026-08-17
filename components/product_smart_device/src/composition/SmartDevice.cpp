#include <board/Board.hpp>
#include <services/BinarySwitchService.hpp>
#include <smart_device/SmartDevice.hpp>
#include <smart_device/SmartDeviceApplication.hpp>

#include "SwitchRuntime.hpp"
#include "NvsBinaryStateStore.hpp"
#if defined(SMART_DEVICE_MATTER_NODE)
#include "MatterNode.hpp"
#endif
#include "nvs_flash.h"

namespace smart_device {
namespace {
constexpr char kFactoryPartition[] = "fctry";

uhal::Status initialize_nvs() {
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (nvs_flash_erase() != ESP_OK) return uhal::Status::io_error;
        error = nvs_flash_init();
    }
    return error == ESP_OK ? uhal::Status::ok : uhal::Status::io_error;
}

uhal::Status initialize_factory_nvs() {
    return nvs_flash_init_partition(kFactoryPartition) == ESP_OK
               ? uhal::Status::ok
               : uhal::Status::io_error;
}

}  // namespace

uhal::Status start() {
    if (initialize_nvs() != uhal::Status::ok) return uhal::Status::io_error;
#if defined(SMART_DEVICE_MATTER_NODE)
    if (initialize_factory_nvs() != uhal::Status::ok) return uhal::Status::io_error;
#endif

    static board::Board board;
    if (board.initialize() != uhal::Status::ok) return uhal::Status::io_error;

    static NvsBinaryStateStore           store;
    static services::BinarySwitchService service{board.relay(), board.led(), store};
#if defined(SMART_DEVICE_MATTER_NODE)
    static MatterNode             matter;
    static SmartDeviceApplication application{service, &matter};
    static SwitchRuntime          runtime{board, application, &matter};
    matter.bind_runtime(runtime);
#else
    static SmartDeviceApplication application{service};
    static SwitchRuntime          runtime{board, application};
#endif

    if (application.initialize() != uhal::Status::ok) return uhal::Status::io_error;
    if (runtime.start() != uhal::Status::ok) return uhal::Status::io_error;
#if defined(SMART_DEVICE_MATTER_NODE)
    return matter.start();
#else
    return uhal::Status::ok;
#endif
}

}  // namespace smart_device
