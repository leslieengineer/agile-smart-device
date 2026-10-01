#pragma once

#include <services/IBinaryStateStore.hpp>
#include <uhal/IGpio.hpp>

namespace services
{

class BinarySwitchService final
{
public:
    BinarySwitchService(uhal::IGpio& load_output, uhal::IGpio& indicator, IBinaryStateStore& store);

    uhal::Status restore();
    uhal::Status set(bool on);
    uhal::Status toggle();
    bool         is_on() const;

private:
    uhal::Status apply();

    uhal::IGpio&       load_output_;
    uhal::IGpio&       indicator_;
    IBinaryStateStore& store_;
    BinaryState        state_{};
};

}  // namespace services
