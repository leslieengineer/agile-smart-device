#include "RhophiClaimProtocol.hpp"

#include <algorithm>

namespace {

class MaterialProvider final : public smart_device::IClaimMaterialProvider {
public:
    uhal::Status load(smart_device::ClaimMaterial& material) override {
        material.product_id = 1U;
        for (std::size_t index = 0; index < material.claim_id.size(); ++index) {
            material.claim_id[index] = static_cast<std::uint8_t>(index + 1U);
        }
        std::fill(material.secret.begin(), material.secret.end(), 0x5AU);
        return uhal::Status::ok;
    }
};

class ClaimCrypto final : public smart_device::IClaimCrypto {
public:
    uhal::Status random(std::uint8_t* output, std::size_t size) override {
        for (std::size_t index = 0; index < size; ++index) output[index] = next_++;
        return uhal::Status::ok;
    }

    uhal::Status hmac_sha256(const std::uint8_t* key, std::size_t key_size,
                             const std::uint8_t* message, std::size_t message_size,
                             std::uint8_t* output, std::size_t output_size) override {
        if (key_size == 0U || message_size == 0U || output_size != 32U) return uhal::Status::invalid_argument;
        for (std::size_t index = 0; index < output_size; ++index) {
            output[index] = static_cast<std::uint8_t>(key[index % key_size] ^ message[index % message_size]);
        }
        return uhal::Status::ok;
    }

private:
    std::uint8_t next_ = 1U;
};

class StateStore final : public smart_device::IClaimStateStore {
public:
    uhal::Status load(smart_device::ClaimPersistentState& state) override {
        state = state_;
        return uhal::Status::ok;
    }
    uhal::Status save(const smart_device::ClaimPersistentState& state) override {
        state_ = state;
        return uhal::Status::ok;
    }
    uhal::Status clear() override {
        state_ = {};
        return uhal::Status::ok;
    }
    const smart_device::ClaimPersistentState& state() const { return state_; }

private:
    smart_device::ClaimPersistentState state_{};
};

class VectorMaterialProvider final : public smart_device::IClaimMaterialProvider {
public:
    uhal::Status load(smart_device::ClaimMaterial& material) override {
        material.product_id = 0x1234U;
        for (std::size_t index = 0U; index < material.secret.size(); ++index) {
            material.secret[index] = static_cast<std::uint8_t>(index);
        }
        for (std::size_t index = 0U; index < material.claim_id.size(); ++index) {
            material.claim_id[index] = static_cast<std::uint8_t>(0x50U + index);
        }
        return uhal::Status::ok;
    }
};

class CapturingCrypto final : public smart_device::IClaimCrypto {
public:
    uhal::Status random(std::uint8_t* output, std::size_t size) override {
        for (std::size_t index = 0U; index < size; ++index) {
            output[index] = static_cast<std::uint8_t>(0x20U + index);
        }
        return uhal::Status::ok;
    }

    uhal::Status hmac_sha256(const std::uint8_t*, std::size_t,
                             const std::uint8_t* message, std::size_t message_size,
                             std::uint8_t* output, std::size_t output_size) override {
        if (message_size != message_.size() || output_size != proof_.size()) {
            return uhal::Status::invalid_argument;
        }
        std::copy(message, message + message_size, message_.begin());
        std::copy(proof_.begin(), proof_.end(), output);
        return uhal::Status::ok;
    }

    const std::array<std::uint8_t, 64U>& message() const { return message_; }

private:
    std::array<std::uint8_t, 64U> message_{};
    const std::array<std::uint8_t, 32U> proof_{
        0x66U, 0xCDU, 0x0EU, 0xE6U, 0x30U, 0x55U, 0xEFU, 0xFDU,
        0x24U, 0xC5U, 0x2BU, 0x90U, 0x77U, 0x9FU, 0x9AU, 0x43U,
        0xE1U, 0xE1U, 0x53U, 0x28U, 0x44U, 0x60U, 0x49U, 0x80U,
        0x43U, 0x52U, 0x67U, 0xFCU, 0xB4U, 0x51U, 0x70U, 0x27U,
    };
};

bool assembles_shared_claim_vector() {
    VectorMaterialProvider provider;
    CapturingCrypto crypto;
    StateStore store;
    smart_device::RhophiClaimProtocol protocol{provider, crypto, store};
    std::array<std::uint8_t, 32U> challenge{};
    for (std::size_t index = 0U; index < challenge.size(); ++index) {
        challenge[index] = static_cast<std::uint8_t>(0x30U + index);
    }
    std::array<std::uint8_t, 32U> proof{};
    if (protocol.initialize() != uhal::Status::ok ||
        protocol.open_window(100U, 1000U) != uhal::Status::ok ||
        protocol.respond(challenge, 200U, proof) != uhal::Status::ok) {
        return false;
    }

    std::array<std::uint8_t, 64U> expected{};
    for (std::size_t index = 0U; index < 16U; ++index) expected[index] = static_cast<std::uint8_t>(0x20U + index);
    for (std::size_t index = 0U; index < 32U; ++index) expected[16U + index] = static_cast<std::uint8_t>(0x30U + index);
    for (std::size_t index = 0U; index < 16U; ++index) expected[48U + index] = static_cast<std::uint8_t>(0x50U + index);
    return crypto.message() == expected && proof[0] == 0x66U && proof[31] == 0x27U;
}

bool requires_physical_window() {
    MaterialProvider provider;
    ClaimCrypto crypto;
    StateStore store;
    smart_device::RhophiClaimProtocol protocol{provider, crypto, store};
    std::array<std::uint8_t, 32U> challenge{};
    std::array<std::uint8_t, 32U> proof{};
    return protocol.initialize() == uhal::Status::ok &&
           protocol.respond(challenge, 100U, proof) == uhal::Status::denied;
}

bool creates_proof_and_rejects_replay() {
    MaterialProvider provider;
    ClaimCrypto crypto;
    StateStore store;
    smart_device::RhophiClaimProtocol protocol{provider, crypto, store};
    std::array<std::uint8_t, 32U> challenge{};
    challenge[0] = 0xA5U;
    std::array<std::uint8_t, 32U> proof{};
    if (protocol.initialize() != uhal::Status::ok ||
        protocol.open_window(100U, 1000U) != uhal::Status::ok) {
        return false;
    }
    const auto identity = protocol.identity(100U);
    if ((identity.flags & 1U) == 0U || identity.product_id != 1U) return false;
    if (protocol.respond(challenge, 200U, proof) != uhal::Status::ok) return false;
    if (std::all_of(proof.begin(), proof.end(), [](std::uint8_t value) { return value == 0U; })) return false;
    return protocol.respond(challenge, 300U, proof) == uhal::Status::denied;
}

bool expires_and_cancels() {
    MaterialProvider provider;
    ClaimCrypto crypto;
    StateStore store;
    smart_device::RhophiClaimProtocol protocol{provider, crypto, store};
    if (protocol.initialize() != uhal::Status::ok ||
        protocol.open_window(100U, 50U) != uhal::Status::ok ||
        !protocol.is_active(149U) || protocol.is_active(150U)) {
        return false;
    }
    if (protocol.open_window(200U, 100U) != uhal::Status::ok) return false;
    protocol.cancel();
    return !protocol.is_active(201U) && (protocol.identity(201U).flags & 1U) == 0U;
}

bool persists_lockout_across_reboot() {
    MaterialProvider provider;
    ClaimCrypto crypto;
    StateStore store;
    {
        smart_device::RhophiClaimProtocol protocol{provider, crypto, store};
        if (protocol.initialize() != uhal::Status::ok ||
            protocol.open_window(100U, 1000U) != uhal::Status::ok) {
            return false;
        }
        for (std::uint8_t attempt = 0U; attempt < 5U; ++attempt) {
            std::array<std::uint8_t, 32U> challenge{};
            challenge[0] = static_cast<std::uint8_t>(attempt + 1U);
            std::array<std::uint8_t, 32U> proof{};
            if (protocol.respond(challenge, static_cast<std::uint32_t>(200U + attempt), proof) !=
                uhal::Status::ok) {
                return false;
            }
        }
    }
    smart_device::RhophiClaimProtocol rebooted{provider, crypto, store};
    return rebooted.initialize() == uhal::Status::ok &&
           rebooted.open_window(1U, 1000U) == uhal::Status::busy &&
           rebooted.open_window(60000U, 1000U) == uhal::Status::ok;
}

bool persists_commissioned_and_clears_factory_state() {
    MaterialProvider provider;
    ClaimCrypto crypto;
    StateStore store;
    smart_device::RhophiClaimProtocol protocol{provider, crypto, store};
    if (protocol.initialize() != uhal::Status::ok ||
        protocol.mark_commissioned() != uhal::Status::ok) {
        return false;
    }
    const std::uint8_t claimed_flags = protocol.identity(0U).flags;
    if ((claimed_flags & 4U) == 0U || (claimed_flags & 2U) != 0U || !store.state().claimed) {
        return false;
    }
    if (protocol.factory_reset() != uhal::Status::ok) return false;
    const std::uint8_t reset_flags = protocol.identity(0U).flags;
    return (reset_flags & 4U) == 0U && (reset_flags & 2U) != 0U && !store.state().claimed;
}

}  // namespace

int main() {
    return assembles_shared_claim_vector() && requires_physical_window() &&
                   creates_proof_and_rejects_replay() && expires_and_cancels() &&
                   persists_lockout_across_reboot() &&
                   persists_commissioned_and_clears_factory_state()
               ? 0
               : 1;
}
