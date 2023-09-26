#pragma once

#include <nil/service/IService.hpp>

#include <memory>

namespace nil::service::tcp
{
    class Server final: public IService
    {
    public:
        struct Options final
        {
            std::uint16_t port;
            /**
             * @brief buffer size to use:
             *  - one each connection for receiving
             */
            std::uint64_t buffer = 1024;
        };

        Server(Options options);
        ~Server() noexcept override;

        void start() override;
        void stop() override;

        void on(
            std::uint32_t type,
            MsgHandler handler //
        ) override;
        void on(
            Event event,
            EventHandler handler //
        ) override;

        void send(
            std::uint16_t id,
            std::uint32_t type,
            const void* data,
            std::uint64_t size //
        ) override;
        void publish(
            std::uint32_t type,
            const void* data,
            std::uint64_t size //
        ) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
