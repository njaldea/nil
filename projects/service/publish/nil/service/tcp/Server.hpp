#pragma once

#include <nil/service/IService.hpp>

namespace nil::service::tcp
{
    class Server final: public IService
    {
    public:
        struct Options
        {
            std::uint16_t port;
        };

        Server(Options options);
        ~Server() override;

        void on(int type, std::unique_ptr<IHandler> handler) override;
        void start() override;
        void stop() override;
        void publish(int type, std::string msg) override;

    private:
        friend struct Connection;
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
