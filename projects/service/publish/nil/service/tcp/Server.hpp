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
            using Service = Server;

            std::uint16_t port;
            /**
             * @brief buffer size to use:
             *  - one for receiving per connection
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

        void on_message(MessageHandler handler) override;
        void on_connect(ConnectHandler handler) override;
        void on_disconnect(DisconnectHandler handler) override;

        void send(const std::string& id, const void* data, std::uint64_t size) override;
        void publish(const void* data, std::uint64_t size) override;

    private:
        detail::Storage<Options> storage;

        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}
