#pragma once

#include <nil/service/IService.hpp>

#include <chrono>
#include <memory>

namespace nil::service::ws
{
    class Client final: public IService
    {
    public:
        struct Options final
        {
            std::string host;
            std::uint16_t port;
            /**
             * @brief buffer size to use:
             *  - one for receiving
             */
            std::uint64_t buffer = 1024;
        };

        Client(Options options);
        ~Client() noexcept override;

        Client(Client&&) = delete;
        Client(const Client&) = delete;
        Client& operator=(Client&&) = delete;
        Client& operator=(const Client&) = delete;

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
