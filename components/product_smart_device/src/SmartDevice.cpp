#include <smart_device/SmartDevice.hpp>

namespace smart_device {

uhal::Status start() {
    // Concrete adapters and services will be composed here.
    return uhal::Status::ok;
}

}  // namespace smart_device
