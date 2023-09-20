#pragma once

#include <string>

namespace nil::service
{
    class IHandler
    {
    public:
        virtual ~IHandler() = default;
        virtual void exec(const std::string& message) = 0;
    };
}
