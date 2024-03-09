#pragma once

#include "../IService.hpp"

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
             *  - one for receiving per connection
             */
            std::uint64_t buffer = 1024;
        };

        explicit Server(Options options);
        ~Server() noexcept override;

        Server(Server&&) = delete;
        Server(const Server&) = delete;
        Server& operator=(Server&&) = delete;
        Server& operator=(const Server&) = delete;

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
