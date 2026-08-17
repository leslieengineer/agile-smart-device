#include "RhophiClaimPlatform.hpp"

#include <algorithm>
#include <array>

#include "esp_random.h"
#include "esp_mac.h"
#include "mbedtls/md.h"
#include "nvs.h"

namespace smart_device {
namespace {
constexpr char kFactoryPartition[] = "fctry";
constexpr char kNamespace[] = "rhophi";
constexpr char kProductIdKey[] = "product_id";
constexpr char kClaimIdKey[] = "claim_id";
constexpr char kClaimSecretKey[] = "claim_secret";
#ifdef RHOPHI_CLAIM_DEV_BYPASS
constexpr char kDevClaimIdContext[] = "rhophi-claim-id-v1:";

uhal::Status load_dev_material(ClaimMaterial& material) {
    std::array<std::uint8_t, 6U> mac{};
    if (esp_efuse_mac_get_default(mac.data()) != ESP_OK) return uhal::Status::io_error;

    std::array<std::uint8_t, (sizeof(kDevClaimIdContext) - 1U) + 6U> input{};
    std::copy_n(reinterpret_cast<const std::uint8_t*>(kDevClaimIdContext),
                sizeof(kDevClaimIdContext) - 1U, input.begin());
    std::copy(mac.begin(), mac.end(), input.begin() + sizeof(kDevClaimIdContext) - 1U);

    std::array<std::uint8_t, 32U> digest{};
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (info == nullptr || mbedtls_md(info, input.data(), input.size(), digest.data()) != 0) {
        return uhal::Status::io_error;
    }

    material.product_id = 0x8000U;
    std::copy_n(digest.begin(), material.claim_id.size(), material.claim_id.begin());
    material.secret.fill(0U);
    return uhal::Status::ok;
}
#endif
}

uhal::Status NvsClaimMaterialProvider::load(ClaimMaterial& material) {
    nvs_handle_t handle{};
    if (nvs_open_from_partition(kFactoryPartition, kNamespace, NVS_READONLY, &handle) != ESP_OK) {
#ifdef RHOPHI_CLAIM_DEV_BYPASS
        return load_dev_material(material);
#else
        return uhal::Status::not_found;
#endif
    }

    std::size_t claim_id_size = material.claim_id.size();
    std::size_t secret_size = material.secret.size();
    const esp_err_t product_error = nvs_get_u16(handle, kProductIdKey, &material.product_id);
    const esp_err_t claim_error = nvs_get_blob(handle, kClaimIdKey, material.claim_id.data(), &claim_id_size);
    const esp_err_t secret_error = nvs_get_blob(handle, kClaimSecretKey, material.secret.data(), &secret_size);
    nvs_close(handle);

    if (product_error != ESP_OK || claim_error != ESP_OK || secret_error != ESP_OK ||
        claim_id_size != material.claim_id.size() || secret_size != material.secret.size()) {
        std::fill(material.secret.begin(), material.secret.end(), 0U);
#ifdef RHOPHI_CLAIM_DEV_BYPASS
        return load_dev_material(material);
#else
        return uhal::Status::corrupt;
#endif
    }
    return uhal::Status::ok;
}

uhal::Status EspClaimCrypto::random(std::uint8_t* output, std::size_t size) {
    if (output == nullptr || size == 0U) return uhal::Status::invalid_argument;
    esp_fill_random(output, size);
    return uhal::Status::ok;
}

uhal::Status EspClaimCrypto::hmac_sha256(const std::uint8_t* key, std::size_t key_size,
                                         const std::uint8_t* message, std::size_t message_size,
                                         std::uint8_t* output, std::size_t output_size) {
    if (key == nullptr || message == nullptr || output == nullptr || output_size != 32U) {
        return uhal::Status::invalid_argument;
    }
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (info == nullptr) return uhal::Status::unsupported;
    return mbedtls_md_hmac(info, key, key_size, message, message_size, output) == 0
               ? uhal::Status::ok
               : uhal::Status::io_error;
}

}  // namespace smart_device
