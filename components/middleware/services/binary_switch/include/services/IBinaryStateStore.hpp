#pragma once

#include <uhal/Status.hpp>

namespace services
{

struct BinaryState
{
    bool on = false;
};

class IBinaryStateStore
{
public:
    virtual ~IBinaryStateStore() = default;

    virtual uhal::Status load(BinaryState& state)       = 0;
    virtual uhal::Status save(const BinaryState& state) = 0;
};

}  // namespace services
