#pragma once

namespace services
{

class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void info(const char* message)  = 0;
    virtual void error(const char* message) = 0;
};

}  // namespace services
