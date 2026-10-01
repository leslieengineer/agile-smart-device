#pragma once

#include <array>
#include <cstddef>

namespace libraries
{

template <typename Event>
class IEventSink
{
public:
    virtual ~IEventSink() = default;

    virtual bool try_push(const Event& event) = 0;
};

template <typename Event, std::size_t Capacity>
class EventQueue final : public IEventSink<Event>
{
public:
    static_assert(Capacity > 0, "EventQueue capacity must be positive");

    bool try_push(const Event& event) override
    {
        if (size_ == Capacity)
        {
            return false;
        }

        events_[write_index_] = event;
        write_index_          = (write_index_ + 1U) % Capacity;
        ++size_;
        return true;
    }

    bool try_pop(Event& event)
    {
        if (size_ == 0)
        {
            return false;
        }

        event       = events_[read_index_];
        read_index_ = (read_index_ + 1U) % Capacity;
        --size_;
        return true;
    }

    std::size_t size() const
    {
        return size_;
    }

private:
    std::array<Event, Capacity> events_{};
    std::size_t                 read_index_  = 0;
    std::size_t                 write_index_ = 0;
    std::size_t                 size_        = 0;
};

template <typename Topic, typename Event, std::size_t TopicCount, std::size_t SubscribersPerTopic>
class EventBus final
{
public:
    static_assert(TopicCount > 0, "EventBus must support at least one topic");
    static_assert(SubscribersPerTopic > 0, "EventBus must support at least one subscriber");

    bool subscribe(Topic topic, IEventSink<Event>& sink)
    {
        const std::size_t topic_index = static_cast<std::size_t>(topic);
        if (topic_index >= TopicCount)
        {
            return false;
        }

        for (IEventSink<Event>*& subscriber : subscribers_[topic_index])
        {
            if (subscriber == &sink)
            {
                return true;
            }
            if (subscriber == nullptr)
            {
                subscriber = &sink;
                return true;
            }
        }
        return false;
    }

    bool publish(Topic topic, const Event& event)
    {
        const std::size_t topic_index = static_cast<std::size_t>(topic);
        if (topic_index >= TopicCount)
        {
            return false;
        }

        bool delivered = false;
        for (IEventSink<Event>* subscriber : subscribers_[topic_index])
        {
            if (subscriber != nullptr && subscriber->try_push(event))
            {
                delivered = true;
            }
        }
        return delivered;
    }

private:
    std::array<std::array<IEventSink<Event>*, SubscribersPerTopic>, TopicCount> subscribers_{};
};

}  // namespace libraries
