#pragma once

#include <array>
#include <cstddef>

namespace libraries
{

template <typename State, typename Event, typename Context, std::size_t TransitionCount>
class StateMachine final
{
public:
    using Action = void (*)(Context& context);

    struct Transition
    {
        State  current_state;
        Event  event;
        State  next_state;
        Action action;
    };

    StateMachine(State initial_state, const std::array<Transition, TransitionCount>& transitions)
        : current_state_(initial_state), transitions_(transitions)
    {
    }

    bool process(Event event, Context& context)
    {
        for (const Transition& transition : transitions_)
        {
            if (transition.current_state != current_state_ || transition.event != event)
            {
                continue;
            }

            current_state_ = transition.next_state;
            if (transition.action != nullptr)
            {
                transition.action(context);
            }
            return true;
        }
        return false;
    }

    State state() const
    {
        return current_state_;
    }

private:
    State                                          current_state_;
    const std::array<Transition, TransitionCount>& transitions_;
};

}  // namespace libraries
