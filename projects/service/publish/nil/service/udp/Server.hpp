#pragma once

#include <nil/service/IService.hpp>

#include <chrono>
#include <memory>

namespace nil::service::udp
{
    class Server final: public IService
    {
    public:
        struct Options
        {
            std::uint16_t port;
            std::uint64_t buffer = 1024;
            std::chrono::microseconds timeout = std::chrono::seconds(2);
        };

        Server(Options options);
        ~Server() override;

        void on(std::uint32_t type, MsgHandler handler) override;
        void on(Event event, EventHandler handler) override;

        void start() override;
        void stop() override;
        void publish(std::uint32_t type, const void* data, std::uint64_t size) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
