#pragma once

#include <nil/service/IService.hpp>

namespace nil::service::tcp
{
    class Client final: public IService
    {
    public:
        struct Options
        {
            std::string host;
            std::uint16_t port;
        };

        Client(Options options);
        ~Client() noexcept override;

        void on(int type, std::unique_ptr<IHandler> handler) override;
        void start() override;
        void stop() override;
        void publish(int type, std::string msg) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
