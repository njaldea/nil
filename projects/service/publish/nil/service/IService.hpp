#pragma once

#include <nil/service/types.hpp>

namespace nil::service
{

    class IService
    {
    public:
        virtual ~IService() noexcept = default;

        virtual void on(std::uint32_t type, MsgHandler handler) = 0;
        virtual void on(Event event, EventHandler handler) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void publish(std::uint32_t type, const void* data, std::uint64_t size) = 0;
    };
}
