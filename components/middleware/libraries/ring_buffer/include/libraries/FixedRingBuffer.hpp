#pragma once

#include <cstddef>

namespace libraries
{

template <typename T, std::size_t Capacity>
class FixedRingBuffer final
{
    static_assert(Capacity > 0U, "Capacity must be greater than zero");

public:
    bool try_push(const T& value)
    {
        if (count_ == Capacity)
            return false;
        entries_[write_index_] = value;
        write_index_           = (write_index_ + 1U) % Capacity;
        ++count_;
        return true;
    }

    bool try_pop(T& value)
    {
        if (count_ == 0U)
            return false;
        value       = entries_[read_index_];
        read_index_ = (read_index_ + 1U) % Capacity;
        --count_;
        return true;
    }

    bool peek(T& value) const
    {
        if (count_ == 0U)
            return false;
        value = entries_[read_index_];
        return true;
    }

    bool drop_oldest()
    {
        if (count_ == 0U)
            return false;
        read_index_ = (read_index_ + 1U) % Capacity;
        --count_;
        return true;
    }

    void clear()
    {
        read_index_  = 0U;
        write_index_ = 0U;
        count_       = 0U;
    }

    std::size_t size() const
    {
        return count_;
    }
    static constexpr std::size_t capacity()
    {
        return Capacity;
    }
    bool empty() const
    {
        return count_ == 0U;
    }
    bool full() const
    {
        return count_ == Capacity;
    }

private:
    T           entries_[Capacity]{};
    std::size_t read_index_  = 0U;
    std::size_t write_index_ = 0U;
    std::size_t count_       = 0U;
};

}  // namespace libraries
