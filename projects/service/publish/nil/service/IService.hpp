#pragma once

#include <nil/service/IHandler.hpp>

#include <memory>

namespace nil::service
{
    class IService
    {
    public:
        virtual ~IService() noexcept = default;

        virtual void on(std::uint32_t type, std::unique_ptr<IHandler> handler) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void publish(std::uint32_t type, std::string msg) = 0;
    };
}
