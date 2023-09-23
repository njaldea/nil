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
            std::uint64_t buffer = 1024;
        };

        Client(Options options);
        ~Client() noexcept override;

        void on(std::uint32_t type, std::unique_ptr<IHandler> handler) override;
        void start() override;
        void stop() override;
        void publish(std::uint32_t type, std::string msg) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
