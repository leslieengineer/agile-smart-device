#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <libraries/Crc16.hpp>
#include <uhal/Result.hpp>

namespace protocols
{

template <std::size_t MaximumPayload>
class FrameCodec final
{
public:
    static constexpr std::uint16_t kSyncWord     = 0xA55AU;
    static constexpr std::size_t   kHeaderSize   = 5U;
    static constexpr std::size_t   kCrcSize      = 2U;
    static constexpr std::size_t   kMaximumFrame = kHeaderSize + MaximumPayload + kCrcSize;

    struct Frame
    {
        std::uint8_t                             opcode = 0;
        std::array<std::uint8_t, MaximumPayload> payload{};
        std::size_t                              payload_size = 0;
    };

    static uhal::Result<std::size_t> encode(const Frame& frame, std::uint8_t* output,
                                            std::size_t output_capacity)
    {
        if (output == nullptr || frame.payload_size > MaximumPayload)
        {
            return uhal::Result<std::size_t>::failure(uhal::Status::invalid_argument);
        }

        const std::size_t frame_size = kHeaderSize + frame.payload_size + kCrcSize;
        if (output_capacity < frame_size)
        {
            return uhal::Result<std::size_t>::failure(uhal::Status::invalid_argument);
        }

        output[0] = static_cast<std::uint8_t>(kSyncWord >> 8U);
        output[1] = static_cast<std::uint8_t>(kSyncWord);
        output[2] = static_cast<std::uint8_t>(frame.payload_size);
        output[3] = frame.opcode;
        for (std::size_t index = 0; index < frame.payload_size; ++index)
        {
            output[4U + index] = frame.payload[index];
        }

        const std::uint16_t crc = libraries::crc16_modbus(output, kHeaderSize + frame.payload_size);
        output[kHeaderSize + frame.payload_size]      = static_cast<std::uint8_t>(crc);
        output[kHeaderSize + frame.payload_size + 1U] = static_cast<std::uint8_t>(crc >> 8U);
        return uhal::Result<std::size_t>::success(frame_size);
    }

    static uhal::Result<Frame> decode(const std::uint8_t* input, std::size_t input_size)
    {
        if (input == nullptr || input_size < kHeaderSize + kCrcSize)
        {
            return uhal::Result<Frame>::failure(uhal::Status::invalid_argument);
        }

        const std::uint16_t sync_word = static_cast<std::uint16_t>(input[0] << 8U) | input[1];
        if (sync_word != kSyncWord)
        {
            return uhal::Result<Frame>::failure(uhal::Status::nack);
        }

        const std::size_t payload_size = input[2];
        const std::size_t frame_size   = kHeaderSize + payload_size + kCrcSize;
        if (payload_size > MaximumPayload || input_size != frame_size)
        {
            return uhal::Result<Frame>::failure(uhal::Status::invalid_argument);
        }

        const std::uint16_t expected_crc = static_cast<std::uint16_t>(input[frame_size - 2U]) |
                                           static_cast<std::uint16_t>(input[frame_size - 1U] << 8U);
        if (libraries::crc16_modbus(input, frame_size - kCrcSize) != expected_crc)
        {
            return uhal::Result<Frame>::failure(uhal::Status::io_error);
        }

        Frame frame{};
        frame.opcode       = input[3];
        frame.payload_size = payload_size;
        for (std::size_t index = 0; index < payload_size; ++index)
        {
            frame.payload[index] = input[4U + index];
        }
        return uhal::Result<Frame>::success(frame);
    }
};

}  // namespace protocols
