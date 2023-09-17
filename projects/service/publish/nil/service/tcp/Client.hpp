#pragma once

#include <nil/service/IService.hpp>

namespace nil::service::tcp
{
    class Client final: public IService
    {
    public:
        Client(std::string host, int port);
        ~Client() noexcept override;

        void on(int type, std::unique_ptr<IHandler> handler) override;
        void start() override;
        void stop() override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
