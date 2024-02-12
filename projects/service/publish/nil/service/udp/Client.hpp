#pragma once

#include "../IService.hpp"

#include <chrono>
#include <memory>

namespace nil::service::udp
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
            /**
             * @brief time to wait until a "connection" is considered as disconnected
             */
            std::chrono::nanoseconds timeout = std::chrono::seconds(2);
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

        void on_message(MessageHandler handler) override;
        void on_connect(ConnectHandler handler) override;
        void on_disconnect(DisconnectHandler handler) override;

        void publish(std::vector<std::uint8_t> data) override;
        void send(const std::string& id, std::vector<std::uint8_t> data) override;

        using IService::publish;
        using IService::publish_raw;
        using IService::send;
        using IService::send_raw;

    private:
        detail::Storage<Options> storage;

        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}
