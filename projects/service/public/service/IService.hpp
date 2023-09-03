#pragma once

#include <memory>

namespace nil::service
{
    class IService
    {
        virtual ~IService() = default;

        virtual void on(int type, std::unique_ptr<IHandler> handler) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
    };
}
