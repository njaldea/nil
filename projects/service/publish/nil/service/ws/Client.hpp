#pragma once

#include "../IService.hpp"

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

        explicit Client(Options options);
        ~Client() noexcept override;

        Client(Client&&) noexcept = delete;
        Client& operator=(Client&&) noexcept = delete;

        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;

        void run() override;
        void stop() override;
        void restart() override;

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

        void on_message_impl(MessageHandler handler) override;
        void on_connect_impl(ConnectHandler handler) override;
        void on_disconnect_impl(DisconnectHandler handler) override;
    };
}
