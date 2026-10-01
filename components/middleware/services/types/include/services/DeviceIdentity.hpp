#pragma once

#include <services/Limits.hpp>

namespace services
{

struct DeviceIdentity
{
    char device_id[kDeviceIdCapacity]{};
    char site_id[kSiteIdCapacity]{};
    char model[16]{};
    char hardware_revision[8]{};
    char firmware_version[16]{};
};

}  // namespace services
