#pragma once

#include <services/IConfigStore.hpp>
#include <services/Limits.hpp>
#include <uhal/IStorage.hpp>

namespace services
{

class ConfigService final : public IConfigStore
{
public:
    explicit ConfigService(uhal::IStorage& storage);

    uhal::Status initialize();
    uhal::Status get_blob(const char* key, std::uint8_t* value, std::size_t capacity,
                          std::size_t& value_size) const override;
    uhal::Status set_blob(const char* key, const std::uint8_t* value,
                          std::size_t value_size) override;
    uhal::Status get_u32(const char* key, std::uint32_t& value) const override;
    uhal::Status set_u32(const char* key, std::uint32_t value) override;
    uhal::Status get_bool(const char* key, bool& value) const override;
    uhal::Status set_bool(const char* key, bool value) override;
    uhal::Status erase(const char* key) override;
    uhal::Status commit() override;

    static constexpr std::uint32_t storage_bytes()
    {
        return kSlotBytes * 2U;
    }

private:
    struct Record
    {
        bool         used = false;
        char         key[kConfigKeyCapacity]{};
        std::uint8_t value[kConfigValueCapacity]{};
        std::uint8_t value_size = 0U;
    };
    struct Snapshot
    {
        std::uint32_t sequence = 0U;
        Record        records[kConfigRecordCapacity]{};
    };

    int          find(const char* key) const;
    int          find_free() const;
    uhal::Status read_slot(std::uint8_t slot, Snapshot& snapshot);
    uhal::Status write_slot(std::uint8_t slot, std::uint32_t sequence);

    static constexpr std::uint32_t kSlotBytes = 2304U;
    uhal::IStorage&                storage_;
    Record                         records_[kConfigRecordCapacity]{};
    std::uint32_t                  sequence_    = 0U;
    std::uint8_t                   active_slot_ = 0U;
    std::uint8_t                   scratch_[kSlotBytes]{};
};

}  // namespace services
