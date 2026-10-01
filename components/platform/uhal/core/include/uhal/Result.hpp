#pragma once

#include <uhal/Status.hpp>

namespace uhal
{

template <typename T>
class Result final
{
public:
    static Result success(const T& value)
    {
        return Result{Status::ok, value};
    }

    static Result failure(Status status)
    {
        return Result{status, T{}};
    }

    bool is_ok() const
    {
        return status_ == Status::ok;
    }

    Status status() const
    {
        return status_;
    }

    const T& value() const
    {
        return value_;
    }

private:
    Result(Status status, const T& value) : status_(status), value_(value) {}

    Status status_;
    T      value_;
};

}  // namespace uhal
