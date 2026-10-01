#include <protocols/MqttTopics.hpp>
namespace protocols
{
namespace
{
bool append(char* out, std::size_t cap, std::size_t& pos, const char* s)
{
    if (s == nullptr)
        return false;
    for (std::size_t i = 0U; s[i] != '\0'; ++i)
    {
        if (pos + 1U >= cap)
            return false;
        out[pos++] = s[i];
    }
    out[pos] = '\0';
    return true;
}
const char* suffix(MqttTopicKind k)
{
    switch (k)
    {
        case MqttTopicKind::status:
            return "status";
        case MqttTopicKind::state:
            return "state";
        case MqttTopicKind::telemetry:
            return "telemetry";
        case MqttTopicKind::command_request:
            return "cmd/req";
        case MqttTopicKind::command_response:
            return "cmd/res";
        case MqttTopicKind::ota_request:
            return "ota/req";
        case MqttTopicKind::ota_event:
            return "ota/evt";
        default:
            return "diag";
    }
}
}  // namespace
uhal::Status MqttTopics::build(services::TopicBuffer& out, MqttTopicKind kind,
                               const char* channel) const
{
    std::size_t p = 0U;
    out.value[0]  = '\0';
    if (!append(out.value, sizeof(out.value), p, "asd/v1/") ||
        !append(out.value, sizeof(out.value), p, site_) ||
        !append(out.value, sizeof(out.value), p, "/") ||
        !append(out.value, sizeof(out.value), p, device_) ||
        !append(out.value, sizeof(out.value), p, "/") ||
        !append(out.value, sizeof(out.value), p, suffix(kind)))
        return uhal::Status::no_resources;
    if (channel != nullptr && (kind == MqttTopicKind::state || kind == MqttTopicKind::telemetry))
    {
        if (!append(out.value, sizeof(out.value), p, "/") ||
            !append(out.value, sizeof(out.value), p, channel))
            return uhal::Status::no_resources;
    }
    return uhal::Status::ok;
}
}  // namespace protocols
