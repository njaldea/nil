#pragma once

#include <nil/service/IService.hpp>

#include <chrono>
#include <memory>

namespace nil::service::ws
{
    class Server final: public IService
    {
    public:
        struct Options final
        {
            std::uint16_t port;
            /**
             * @brief buffer size to use:
             *  - one for receiving
             */
            std::uint64_t buffer = 1024;
        };

        Server(Options options);
        ~Server() noexcept override;

        Server(Server&&) = delete;
        Server(const Server&) = delete;
        Server& operator=(Server&&) = delete;
        Server& operator=(const Server&) = delete;

        void run() override;
        void stop() override;
        void restart() override;

        void on(
            std::uint32_t type,
            MsgHandler handler //
        ) override;
        void on(
            Event event,
            EventHandler handler //
        ) override;

        void send(
            const std::string& id,
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
        detail::Storage<Options> storage;

        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}
