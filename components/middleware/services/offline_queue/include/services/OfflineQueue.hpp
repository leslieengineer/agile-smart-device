#pragma once

#include <libraries/FixedRingBuffer.hpp>
#include <services/IOfflineQueue.hpp>

namespace services
{

template <std::size_t Depth = kOfflineQueueDepth>
class OfflineQueue final : public IOfflineQueue
{
public:
    uhal::Status enqueue(const MessageEnvelope& message, bool allow_drop) override
    {
        if (message.payload.size > kPayloadCapacity)
            return uhal::Status::no_resources;
        std::size_t topic_size = 0U;
        while (topic_size < kTopicCapacity && message.topic.value[topic_size] != '\0') ++topic_size;
        if (topic_size >= kTopicCapacity)
            return uhal::Status::no_resources;

        QueuedMessage queued{};
        for (std::size_t index = 0U; index < topic_size; ++index)
            queued.topic[index] = message.topic.value[index];
        for (std::size_t index = 0U; index < message.payload.size; ++index)
            queued.payload[index] = message.payload.data[index];
        queued.payload_size   = static_cast<std::uint16_t>(message.payload.size);
        queued.qos            = message.qos;
        queued.retain         = message.retain;
        queued.correlation_id = message.correlation_id;

        if (queue_.try_push(queued))
            return uhal::Status::ok;
        if (!allow_drop)
            return uhal::Status::no_resources;
        queue_.drop_oldest();
        ++dropped_;
        return queue_.try_push(queued) ? uhal::Status::ok : uhal::Status::no_resources;
    }

    bool peek(QueuedMessage& message) const override
    {
        return queue_.peek(message);
    }
    void commit_front() override
    {
        queue_.drop_oldest();
    }
    std::size_t size() const override
    {
        return queue_.size();
    }
    std::uint32_t dropped_count() const
    {
        return dropped_;
    }

private:
    libraries::FixedRingBuffer<QueuedMessage, Depth> queue_{};
    std::uint32_t                                    dropped_ = 0U;
};

}  // namespace services
