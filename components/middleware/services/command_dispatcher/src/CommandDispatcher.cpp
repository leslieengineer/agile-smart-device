#include <libraries/JsonScanner.hpp>
#include <libraries/JsonWriter.hpp>
#include <services/CommandDispatcher.hpp>
namespace services
{
namespace
{
bool equal(const char* a, const char* b, std::size_t cap)
{
    if (a == nullptr || b == nullptr)
        return false;
    for (std::size_t i = 0U; i < cap; ++i)
    {
        if (a[i] != b[i])
            return false;
        if (a[i] == '\0')
            return true;
    }
    return false;
}
void copy(char* out, std::size_t cap, const char* in)
{
    std::size_t i = 0U;
    for (; i + 1U < cap && in != nullptr && in[i] != '\0'; ++i) out[i] = in[i];
    out[i] = '\0';
}
}  // namespace
CommandDispatcher::CommandDispatcher(IMessageTransport& t, IOfflineQueue& q, ICommandAuthorizer& a,
                                     const protocols::MqttTopics& topics)
    : transport_{t}, queue_{q}, authorizer_{a}, topics_{topics}
{
}
uhal::Status CommandDispatcher::add_handler(ICommandHandler& h)
{
    if (handler_count_ >= kCommandHandlerCapacity)
        return uhal::Status::no_resources;
    handlers_[handler_count_++] = &h;
    return uhal::Status::ok;
}
void CommandDispatcher::on_message(const MessageEnvelope& m)
{
    CommandRequest         request{};
    CommandResponse        response{};
    libraries::JsonScanner scanner{m.payload.data, m.payload.size};
    if (!scanner.get_string("id", request.id, sizeof(request.id)) ||
        !scanner.get_string("name", request.name, sizeof(request.name)))
    {
        response.status = uhal::Status::corrupt;
        respond("", response, m.correlation_id);
        return;
    }
    if (scanner.get_bool("value", request.value))
        request.has_value = true;
    if (find_cached(request.id, response))
    {
        respond(request.id, response, m.correlation_id);
        return;
    }
    response.status = authorizer_.authorize(request);
    if (response.status == uhal::Status::ok)
    {
        ICommandHandler* h = find_handler(request.name);
        response.status    = h == nullptr ? uhal::Status::not_found : h->handle(request, response);
    }
    cache(request.id, response);
    respond(request.id, response, m.correlation_id);
}
ICommandHandler* CommandDispatcher::find_handler(const char* name) const
{
    for (std::size_t i = 0U; i < handler_count_; ++i)
        if (equal(handlers_[i]->name(), name, 24U))
            return handlers_[i];
    return nullptr;
}
bool CommandDispatcher::find_cached(const char* id, CommandResponse& out) const
{
    for (const auto& item : cache_)
        if (item.used && equal(item.id, id, 16U))
        {
            out = item.response;
            return true;
        }
    return false;
}
void CommandDispatcher::cache(const char* id, const CommandResponse& r)
{
    Cache& item = cache_[cache_index_];
    item        = Cache{};
    item.used   = true;
    copy(item.id, sizeof(item.id), id);
    item.response = r;
    cache_index_  = (cache_index_ + 1U) % kIdempotencyWindow;
}
void CommandDispatcher::respond(const char* id, const CommandResponse& r, std::uint32_t correlation)
{
    std::uint8_t          payload[kPayloadCapacity]{};
    libraries::JsonWriter writer{payload, sizeof(payload)};
    if (!writer.begin_object() || !writer.field_string("id", id) ||
        !writer.field_u32("status", static_cast<std::uint32_t>(r.status)) ||
        !writer.field_bool("state", r.state) || !writer.end_object())
        return;
    MessageEnvelope message{};
    if (topics_.build(message.topic, protocols::MqttTopicKind::command_response) !=
        uhal::Status::ok)
        return;
    message.payload        = {payload, writer.size()};
    message.qos            = QosLevel::at_least_once;
    message.correlation_id = correlation;
    if (!transport_.is_connected() || transport_.publish(message) != uhal::Status::ok)
        queue_.enqueue(message, false);
}
}  // namespace services
