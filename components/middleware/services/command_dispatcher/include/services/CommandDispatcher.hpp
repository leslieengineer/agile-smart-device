#pragma once
#include <protocols/MqttTopics.hpp>
#include <services/ICommandHandler.hpp>
#include <services/IOfflineQueue.hpp>
#include <services/Limits.hpp>
namespace services
{
class CommandDispatcher final : public IMessageHandler
{
public:
    CommandDispatcher(IMessageTransport&, IOfflineQueue&, ICommandAuthorizer&,
                      const protocols::MqttTopics&);
    uhal::Status add_handler(ICommandHandler&);
    void         on_message(const MessageEnvelope&) override;

private:
    struct Cache
    {
        bool            used = false;
        char            id[16]{};
        CommandResponse response{};
    };
    ICommandHandler*             find_handler(const char*) const;
    bool                         find_cached(const char*, CommandResponse&) const;
    void                         cache(const char*, const CommandResponse&);
    void                         respond(const char*, const CommandResponse&, std::uint32_t);
    IMessageTransport&           transport_;
    IOfflineQueue&               queue_;
    ICommandAuthorizer&          authorizer_;
    const protocols::MqttTopics& topics_;
    ICommandHandler*             handlers_[kCommandHandlerCapacity]{};
    std::size_t                  handler_count_ = 0U;
    Cache                        cache_[kIdempotencyWindow]{};
    std::size_t                  cache_index_ = 0U;
};
}  // namespace services
