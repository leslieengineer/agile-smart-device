#pragma once
#include <libraries/JsonWriter.hpp>
#include <protocols/MqttTopics.hpp>
#include <services/IOfflineQueue.hpp>
namespace services
{
class TelemetryService final
{
public:
    TelemetryService(IMessageTransport& t, IOfflineQueue& q, const protocols::MqttTopics& topics)
        : transport_{t}, queue_{q}, topics_{topics}
    {
    }
    uhal::Status publish_bool(const char* channel, bool value, std::uint32_t utc_s)
    {
        std::uint8_t          payload[kPayloadCapacity]{};
        libraries::JsonWriter writer{payload, sizeof(payload)};
        if (!writer.begin_object() || !writer.field_u32("ts", utc_s) ||
            !writer.field_u32("seq", sequence_) || !writer.field_bool("value", value) ||
            !writer.end_object())
            return uhal::Status::no_resources;
        MessageEnvelope message{};
        if (topics_.build(message.topic, protocols::MqttTopicKind::telemetry, channel) !=
            uhal::Status::ok)
            return uhal::Status::no_resources;
        message.payload = {payload, writer.size()};
        if (!transport_.is_connected())
        {
            ++sequence_;
            return queue_.enqueue(message, true);
        }
        const uhal::Status status = transport_.publish(message);
        if (status != uhal::Status::ok)
        {
            ++sequence_;
            return queue_.enqueue(message, true);
        }
        ++sequence_;
        return uhal::Status::ok;
    }
    uhal::Status replay_one()
    {
        if (!transport_.is_connected())
            return uhal::Status::not_ready;
        QueuedMessage queued{};
        if (!queue_.peek(queued))
            return uhal::Status::not_found;
        MessageEnvelope message{};
        std::size_t     i = 0U;
        for (; i + 1U < kTopicCapacity && queued.topic[i] != '\0'; ++i)
            message.topic.value[i] = queued.topic[i];
        message.payload           = {queued.payload, queued.payload_size};
        message.qos               = queued.qos;
        message.retain            = queued.retain;
        message.correlation_id    = queued.correlation_id;
        const uhal::Status status = transport_.publish(message);
        if (status == uhal::Status::ok)
            queue_.commit_front();
        return status;
    }
    std::uint32_t sequence() const
    {
        return sequence_;
    }

private:
    IMessageTransport&           transport_;
    IOfflineQueue&               queue_;
    const protocols::MqttTopics& topics_;
    std::uint32_t                sequence_ = 0U;
};
}  // namespace services
