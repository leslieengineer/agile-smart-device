#include "RhophiClaimProtocol.hpp"

#include <algorithm>

namespace smart_device {
namespace {

constexpr std::uint8_t kCommissionableFlag = 1U << 0U;
constexpr std::uint8_t kFactoryNewFlag = 1U << 1U;
constexpr std::uint8_t kClaimedFlag = 1U << 2U;
constexpr std::uint8_t kLockedFlag = 1U << 3U;
constexpr std::uint8_t kMaximumAttempts = 5U;
constexpr std::uint32_t kInitialLockoutMs = 60000U;
constexpr std::uint32_t kMaximumLockoutMs = 900000U;
constexpr std::uint8_t kMaximumLockoutLevel = 5U;

}  // namespace

RhophiClaimProtocol::RhophiClaimProtocol(IClaimMaterialProvider& material_provider,
                                         IClaimCrypto& crypto, IClaimStateStore& state_store)
    : material_provider_{material_provider}, crypto_{crypto}, state_store_{state_store} {}

uhal::Status RhophiClaimProtocol::initialize() {
    const uhal::Status material_status = material_provider_.load(material_);
    if (material_status != uhal::Status::ok) return material_status;
    const uhal::Status state_status = state_store_.load(persistent_);
    if (state_status != uhal::Status::ok) {
        std::fill(material_.secret.begin(), material_.secret.end(), 0U);
        return state_status;
    }
    identity_.product_id = material_.product_id;
    identity_.claim_id = material_.claim_id;
    lockout_until_ms_ = persistent_.lockout_ms;
    initialized_ = true;
    refresh_flags(0U);
    return uhal::Status::ok;
}

uhal::Status RhophiClaimProtocol::open_window(std::uint32_t now_ms, std::uint32_t duration_ms) {
    if (!initialized_) return uhal::Status::not_ready;
    if (duration_ms == 0U) return uhal::Status::invalid_argument;
    if (is_locked(now_ms)) {
        refresh_flags(now_ms);
        return uhal::Status::busy;
    }
    const uhal::Status status = rotate_nonce();
    if (status != uhal::Status::ok) return status;
    expires_ms_ = now_ms + duration_ms;
    replay_count_ = 0U;
    replay_next_ = 0U;
    for (auto& challenge : replay_cache_) std::fill(challenge.begin(), challenge.end(), 0U);
    active_ = true;
    refresh_flags(now_ms);
    return uhal::Status::ok;
}

void RhophiClaimProtocol::cancel() {
    close();
}

bool RhophiClaimProtocol::is_active(std::uint32_t now_ms) {
    if (active_ && static_cast<std::int32_t>(now_ms - expires_ms_) >= 0) close();
    if (is_locked(now_ms)) active_ = false;
    refresh_flags(now_ms);
    return active_;
}

ClaimIdentity RhophiClaimProtocol::identity(std::uint32_t now_ms) {
    is_active(now_ms);
    return identity_;
}

uhal::Status RhophiClaimProtocol::respond(const std::array<std::uint8_t, 32U>& challenge,
                                          std::uint32_t now_ms,
                                          std::array<std::uint8_t, 32U>& proof) {
    std::fill(proof.begin(), proof.end(), 0U);
    if (!is_active(now_ms)) return uhal::Status::denied;
    if (is_locked(now_ms) || persistent_.attempts >= kMaximumAttempts) return uhal::Status::busy;
    if (challenge_seen(challenge)) return uhal::Status::denied;

#ifdef RHOPHI_CLAIM_DEV_BYPASS
    const uhal::Status dev_nonce_status = rotate_nonce();
    if (dev_nonce_status != uhal::Status::ok) return dev_nonce_status;
    std::fill(proof.begin(), proof.end(), 0U);
    remember_challenge(challenge);
    return uhal::Status::ok;
#endif

    std::array<std::uint8_t, 64U> message{};
    auto output = message.begin();
    output = std::copy(identity_.nonce.begin(), identity_.nonce.end(), output);
    output = std::copy(challenge.begin(), challenge.end(), output);
    std::copy(identity_.claim_id.begin(), identity_.claim_id.end(), output);

    const uhal::Status status = crypto_.hmac_sha256(
        material_.secret.data(), material_.secret.size(), message.data(), message.size(),
        proof.data(), proof.size());
    std::fill(message.begin(), message.end(), 0U);
    if (status != uhal::Status::ok) return status;

    const uhal::Status nonce_status = rotate_nonce();
    if (nonce_status != uhal::Status::ok) {
        std::fill(proof.begin(), proof.end(), 0U);
        return nonce_status;
    }

    remember_challenge(challenge);
    persistent_.attempts += 1U;
    if (persistent_.attempts >= kMaximumAttempts) {
        persistent_.lockout_level = std::min<std::uint8_t>(
            static_cast<std::uint8_t>(persistent_.lockout_level + 1U), kMaximumLockoutLevel);
        std::uint32_t duration = kInitialLockoutMs;
        for (std::uint8_t level = 1U; level < persistent_.lockout_level; ++level) {
            duration = std::min(duration * 2U, kMaximumLockoutMs);
        }
        persistent_.lockout_ms = duration;
        lockout_until_ms_ = now_ms + duration;
        active_ = false;
    }
    const uhal::Status persist_status = persist_state();
    refresh_flags(now_ms);
    if (persist_status != uhal::Status::ok) {
        std::fill(proof.begin(), proof.end(), 0U);
        return persist_status;
    }
    return uhal::Status::ok;
}

uhal::Status RhophiClaimProtocol::mark_commissioned() {
    if (!initialized_) return uhal::Status::not_ready;
    persistent_.claimed = true;
    persistent_.attempts = 0U;
    persistent_.lockout_level = 0U;
    persistent_.lockout_ms = 0U;
    lockout_until_ms_ = 0U;
    close();
    return persist_state();
}

uhal::Status RhophiClaimProtocol::factory_reset() {
    close();
    persistent_ = {};
    lockout_until_ms_ = 0U;
    refresh_flags(0U);
    return state_store_.clear();
}

void RhophiClaimProtocol::close() {
    active_ = false;
    expires_ms_ = 0U;
    clear_transient();
    refresh_flags(0U);
}

void RhophiClaimProtocol::clear_transient() {
    std::fill(identity_.nonce.begin(), identity_.nonce.end(), 0U);
    for (auto& challenge : replay_cache_) std::fill(challenge.begin(), challenge.end(), 0U);
    replay_count_ = 0U;
    replay_next_ = 0U;
}

void RhophiClaimProtocol::refresh_flags(std::uint32_t now_ms) {
    std::uint8_t flags = persistent_.claimed ? kClaimedFlag : kFactoryNewFlag;
    if (active_) flags |= kCommissionableFlag;
    if (persistent_.lockout_ms != 0U &&
        static_cast<std::int32_t>(now_ms - lockout_until_ms_) < 0) {
        flags |= kLockedFlag;
    }
    identity_.flags = flags;
}

bool RhophiClaimProtocol::is_locked(std::uint32_t now_ms) {
    if (persistent_.lockout_ms == 0U) return false;
    if (static_cast<std::int32_t>(now_ms - lockout_until_ms_) < 0) return true;
    persistent_.attempts = 0U;
    persistent_.lockout_ms = 0U;
    lockout_until_ms_ = 0U;
    (void)persist_state();
    return false;
}

bool RhophiClaimProtocol::challenge_seen(
    const std::array<std::uint8_t, 32U>& challenge) const {
    for (std::size_t index = 0U; index < replay_count_; ++index) {
        if (replay_cache_[index] == challenge) return true;
    }
    return false;
}

void RhophiClaimProtocol::remember_challenge(
    const std::array<std::uint8_t, 32U>& challenge) {
    replay_cache_[replay_next_] = challenge;
    replay_next_ = (replay_next_ + 1U) % kReplayCacheSize;
    replay_count_ = std::min(replay_count_ + 1U, kReplayCacheSize);
}

uhal::Status RhophiClaimProtocol::rotate_nonce() {
    return crypto_.random(identity_.nonce.data(), identity_.nonce.size());
}

uhal::Status RhophiClaimProtocol::persist_state() {
    return state_store_.save(persistent_);
}

}  // namespace smart_device
