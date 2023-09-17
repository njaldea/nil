#pragma once

#include <nil/service/IService.hpp>

namespace nil::service::tcp
{
    class Server final: public IService
    {
    public:
        Server(std::string host, int port);
        ~Server() override;

        void on(int type, std::unique_ptr<IHandler> handler) override;
        void start() override;
        void stop() override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
