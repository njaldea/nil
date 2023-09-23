#pragma once

#include <string>

namespace nil::service
{
    class IHandler
    {
    public:
        virtual ~IHandler() = default;
        virtual void exec(const void* data, std::size_t size) = 0;
    };
}
