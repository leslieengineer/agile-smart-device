#pragma once

#include "RhophiClaimProtocol.hpp"

namespace smart_device {

class NvsClaimMaterialProvider final : public IClaimMaterialProvider {
public:
    uhal::Status load(ClaimMaterial& material) override;
};

class EspClaimCrypto final : public IClaimCrypto {
public:
    uhal::Status random(std::uint8_t* output, std::size_t size) override;
    uhal::Status hmac_sha256(const std::uint8_t* key, std::size_t key_size,
                             const std::uint8_t* message, std::size_t message_size,
                             std::uint8_t* output, std::size_t output_size) override;
};

}  // namespace smart_device
