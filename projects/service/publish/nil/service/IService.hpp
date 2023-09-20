#pragma once

#include <memory>

namespace nil::service
{
    class IHandler
    {
    };

    class IService
    {
    public:
        virtual ~IService() noexcept = default;

        virtual void on(int type, std::unique_ptr<IHandler> handler) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void publish(int type, std::string msg) = 0;
    };
}
