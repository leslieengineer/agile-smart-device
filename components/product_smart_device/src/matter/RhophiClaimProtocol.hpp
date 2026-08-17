#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <uhal/Status.hpp>

namespace smart_device {

struct ClaimMaterial {
    std::uint16_t product_id = 0U;
    std::array<std::uint8_t, 16U> claim_id{};
    std::array<std::uint8_t, 32U> secret{};
};

struct ClaimIdentity {
    std::uint8_t protocol_version = 1U;
    std::uint16_t product_id = 0U;
    std::array<std::uint8_t, 16U> claim_id{};
    std::array<std::uint8_t, 16U> nonce{};
    std::uint8_t flags = 0U;
};

struct ClaimPersistentState {
    std::uint32_t lockout_ms = 0U;
    std::uint8_t attempts = 0U;
    std::uint8_t lockout_level = 0U;
    bool claimed = false;
};

class IClaimMaterialProvider {
public:
    virtual ~IClaimMaterialProvider() = default;
    virtual uhal::Status load(ClaimMaterial& material) = 0;
};

class IClaimCrypto {
public:
    virtual ~IClaimCrypto() = default;
    virtual uhal::Status random(std::uint8_t* output, std::size_t size) = 0;
    virtual uhal::Status hmac_sha256(const std::uint8_t* key, std::size_t key_size,
                                     const std::uint8_t* message, std::size_t message_size,
                                     std::uint8_t* output, std::size_t output_size) = 0;
};

class IClaimStateStore {
public:
    virtual ~IClaimStateStore() = default;
    virtual uhal::Status load(ClaimPersistentState& state) = 0;
    virtual uhal::Status save(const ClaimPersistentState& state) = 0;
    virtual uhal::Status clear() = 0;
};

class RhophiClaimProtocol final {
public:
    RhophiClaimProtocol(IClaimMaterialProvider& material_provider, IClaimCrypto& crypto,
                        IClaimStateStore& state_store);

    uhal::Status initialize();
    uhal::Status open_window(std::uint32_t now_ms, std::uint32_t duration_ms);
    void cancel();
    bool is_active(std::uint32_t now_ms);
    ClaimIdentity identity(std::uint32_t now_ms);
    uhal::Status respond(const std::array<std::uint8_t, 32U>& challenge, std::uint32_t now_ms,
                         std::array<std::uint8_t, 32U>& proof);
    uhal::Status mark_commissioned();
    uhal::Status factory_reset();

private:
    static constexpr std::size_t kReplayCacheSize = 8U;

    void close();
    void clear_transient();
    void refresh_flags(std::uint32_t now_ms);
    bool is_locked(std::uint32_t now_ms);
    bool challenge_seen(const std::array<std::uint8_t, 32U>& challenge) const;
    void remember_challenge(const std::array<std::uint8_t, 32U>& challenge);
    uhal::Status rotate_nonce();
    uhal::Status persist_state();

    IClaimMaterialProvider& material_provider_;
    IClaimCrypto& crypto_;
    IClaimStateStore& state_store_;
    ClaimMaterial material_{};
    ClaimIdentity identity_{};
    ClaimPersistentState persistent_{};
    std::array<std::array<std::uint8_t, 32U>, kReplayCacheSize> replay_cache_{};
    std::uint32_t expires_ms_ = 0U;
    std::uint32_t lockout_until_ms_ = 0U;
    std::size_t replay_count_ = 0U;
    std::size_t replay_next_ = 0U;
    bool initialized_ = false;
    bool active_ = false;
};

}  // namespace smart_device
