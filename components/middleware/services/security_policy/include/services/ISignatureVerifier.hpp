#pragma once

#include <cstddef>
#include <cstdint>
#include <uhal/Status.hpp>

namespace services
{

class ISignatureVerifier
{
public:
    virtual ~ISignatureVerifier()                    = default;
    virtual uhal::Status verify(const std::uint8_t* digest, std::size_t digest_size,
                                const std::uint8_t* signature, std::size_t signature_size,
                                std::uint8_t key_id) = 0;
};

}  // namespace services
