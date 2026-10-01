#include <libraries/ByteReader.hpp>
#include <libraries/ByteWriter.hpp>
#include <libraries/Crc32.hpp>
#include <services/ConfigService.hpp>

namespace services
{
namespace
{

constexpr std::uint32_t kMagic = 0x31474643U;

std::size_t text_length(const char* text, std::size_t capacity)
{
    if (text == nullptr)
        return capacity;
    std::size_t size = 0U;
    while (size < capacity && text[size] != '\0') ++size;
    return size;
}

bool text_equal(const char* left, const char* right, std::size_t capacity)
{
    if (left == nullptr || right == nullptr)
        return false;
    for (std::size_t index = 0U; index < capacity; ++index)
    {
        if (left[index] != right[index])
            return false;
        if (left[index] == '\0')
            return true;
    }
    return false;
}

}  // namespace

ConfigService::ConfigService(uhal::IStorage& storage) : storage_{storage} {}

uhal::Status ConfigService::initialize()
{
    Snapshot           first{};
    Snapshot           second{};
    const uhal::Status first_status  = read_slot(0U, first);
    const uhal::Status second_status = read_slot(1U, second);

    const bool first_valid  = first_status == uhal::Status::ok;
    const bool second_valid = second_status == uhal::Status::ok;
    if (!first_valid && !second_valid)
    {
        if ((first_status != uhal::Status::corrupt && first_status != uhal::Status::not_found) ||
            (second_status != uhal::Status::corrupt && second_status != uhal::Status::not_found))
            return uhal::Status::io_error;
        for (auto& record : records_) record = Record{};
        sequence_    = 0U;
        active_slot_ = 0U;
        return uhal::Status::ok;
    }

    const Snapshot& selected =
        !second_valid || (first_valid && first.sequence >= second.sequence) ? first : second;
    active_slot_ = &selected == &first ? 0U : 1U;
    sequence_    = selected.sequence;
    for (std::size_t index = 0U; index < kConfigRecordCapacity; ++index)
        records_[index] = selected.records[index];
    return uhal::Status::ok;
}

uhal::Status ConfigService::get_blob(const char* key, std::uint8_t* value, std::size_t capacity,
                                     std::size_t& value_size) const
{
    const int index = find(key);
    if (index < 0)
        return uhal::Status::not_found;
    const Record& record = records_[static_cast<std::size_t>(index)];
    value_size           = record.value_size;
    if (value == nullptr || capacity < value_size)
        return uhal::Status::no_resources;
    for (std::size_t offset = 0U; offset < value_size; ++offset)
        value[offset] = record.value[offset];
    return uhal::Status::ok;
}

uhal::Status ConfigService::set_blob(const char* key, const std::uint8_t* value,
                                     std::size_t value_size)
{
    const std::size_t key_size = text_length(key, kConfigKeyCapacity);
    if (key_size == 0U || key_size >= kConfigKeyCapacity || value == nullptr ||
        value_size > kConfigValueCapacity)
        return uhal::Status::invalid_argument;
    int index = find(key);
    if (index < 0)
        index = find_free();
    if (index < 0)
        return uhal::Status::no_resources;

    Record& record = records_[static_cast<std::size_t>(index)];
    record         = Record{};
    record.used    = true;
    for (std::size_t offset = 0U; offset < key_size; ++offset) record.key[offset] = key[offset];
    record.key[key_size] = '\0';
    for (std::size_t offset = 0U; offset < value_size; ++offset)
        record.value[offset] = value[offset];
    record.value_size = static_cast<std::uint8_t>(value_size);
    return uhal::Status::ok;
}

uhal::Status ConfigService::get_u32(const char* key, std::uint32_t& value) const
{
    std::uint8_t       bytes[4]{};
    std::size_t        size   = 0U;
    const uhal::Status status = get_blob(key, bytes, sizeof(bytes), size);
    if (status != uhal::Status::ok)
        return status;
    if (size != sizeof(bytes))
        return uhal::Status::corrupt;
    libraries::ByteReader reader{bytes, sizeof(bytes)};
    return reader.read_u32_le(value) ? uhal::Status::ok : uhal::Status::corrupt;
}

uhal::Status ConfigService::set_u32(const char* key, std::uint32_t value)
{
    std::uint8_t          bytes[4]{};
    libraries::ByteWriter writer{bytes, sizeof(bytes)};
    return writer.write_u32_le(value) ? set_blob(key, bytes, sizeof(bytes))
                                      : uhal::Status::no_resources;
}

uhal::Status ConfigService::get_bool(const char* key, bool& value) const
{
    std::uint8_t       stored = 0U;
    std::size_t        size   = 0U;
    const uhal::Status status = get_blob(key, &stored, 1U, size);
    if (status != uhal::Status::ok)
        return status;
    if (size != 1U || stored > 1U)
        return uhal::Status::corrupt;
    value = stored != 0U;
    return uhal::Status::ok;
}

uhal::Status ConfigService::set_bool(const char* key, bool value)
{
    const std::uint8_t stored = value ? 1U : 0U;
    return set_blob(key, &stored, 1U);
}

uhal::Status ConfigService::erase(const char* key)
{
    const int index = find(key);
    if (index < 0)
        return uhal::Status::not_found;
    records_[static_cast<std::size_t>(index)] = Record{};
    return uhal::Status::ok;
}

uhal::Status ConfigService::commit()
{
    const std::uint8_t target_slot = active_slot_ == 0U ? 1U : 0U;
    const uhal::Status status      = write_slot(target_slot, sequence_ + 1U);
    if (status == uhal::Status::ok)
    {
        active_slot_ = target_slot;
        ++sequence_;
    }
    return status;
}

int ConfigService::find(const char* key) const
{
    for (std::size_t index = 0U; index < kConfigRecordCapacity; ++index)
    {
        if (records_[index].used && text_equal(records_[index].key, key, kConfigKeyCapacity))
            return static_cast<int>(index);
    }
    return -1;
}

int ConfigService::find_free() const
{
    for (std::size_t index = 0U; index < kConfigRecordCapacity; ++index)
    {
        if (!records_[index].used)
            return static_cast<int>(index);
    }
    return -1;
}

uhal::Status ConfigService::read_slot(std::uint8_t slot, Snapshot& snapshot)
{
    const std::uint32_t offset = static_cast<std::uint32_t>(slot) * kSlotBytes;
    const uhal::Status  status = storage_.read(offset, scratch_, sizeof(scratch_));
    if (status != uhal::Status::ok)
        return status;

    const std::size_t     crc_offset = sizeof(scratch_) - 4U;
    libraries::ByteReader crc_reader{scratch_ + crc_offset, 4U};
    std::uint32_t         stored_crc = 0U;
    if (!crc_reader.read_u32_le(stored_crc) || stored_crc != libraries::crc32(scratch_, crc_offset))
        return uhal::Status::corrupt;

    libraries::ByteReader reader{scratch_, crc_offset};
    std::uint32_t         magic = 0U;
    std::uint8_t          count = 0U;
    if (!reader.read_u32_le(magic) || !reader.read_u32_le(snapshot.sequence) ||
        !reader.read_u8(count) || magic != kMagic || count > kConfigRecordCapacity)
        return uhal::Status::corrupt;

    for (std::uint8_t record_index = 0U; record_index < count; ++record_index)
    {
        std::uint8_t key_size   = 0U;
        std::uint8_t value_size = 0U;
        if (!reader.read_u8(key_size) || !reader.read_u8(value_size) || key_size == 0U ||
            key_size >= kConfigKeyCapacity || value_size > kConfigValueCapacity)
            return uhal::Status::corrupt;
        Record& record = snapshot.records[record_index];
        record.used    = true;
        if (!reader.read_bytes(reinterpret_cast<std::uint8_t*>(record.key), key_size) ||
            !reader.read_bytes(record.value, value_size))
            return uhal::Status::corrupt;
        record.key[key_size] = '\0';
        record.value_size    = value_size;
    }
    return uhal::Status::ok;
}

uhal::Status ConfigService::write_slot(std::uint8_t slot, std::uint32_t sequence)
{
    for (auto& byte : scratch_) byte = 0U;
    const std::size_t     crc_offset = sizeof(scratch_) - 4U;
    libraries::ByteWriter writer{scratch_, crc_offset};
    std::uint8_t          count = 0U;
    for (const auto& record : records_)
    {
        if (record.used)
            ++count;
    }
    if (!writer.write_u32_le(kMagic) || !writer.write_u32_le(sequence) || !writer.write_u8(count))
        return uhal::Status::no_resources;
    for (const auto& record : records_)
    {
        if (!record.used)
            continue;
        const std::size_t key_size = text_length(record.key, kConfigKeyCapacity);
        if (!writer.write_u8(static_cast<std::uint8_t>(key_size)) ||
            !writer.write_u8(record.value_size) ||
            !writer.write_bytes(reinterpret_cast<const std::uint8_t*>(record.key), key_size) ||
            !writer.write_bytes(record.value, record.value_size))
            return uhal::Status::no_resources;
    }
    const std::uint32_t   crc = libraries::crc32(scratch_, crc_offset);
    libraries::ByteWriter crc_writer{scratch_ + crc_offset, 4U};
    if (!crc_writer.write_u32_le(crc))
        return uhal::Status::no_resources;

    const std::uint32_t offset       = static_cast<std::uint32_t>(slot) * kSlotBytes;
    const uhal::Status  erase_status = storage_.erase(offset, kSlotBytes);
    if (erase_status != uhal::Status::ok)
        return erase_status;
    return storage_.write(offset, scratch_, sizeof(scratch_));
}

}  // namespace services
