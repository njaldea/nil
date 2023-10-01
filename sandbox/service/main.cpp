#include <nil/cli.hpp>
#include <nil/service.hpp>

#include <iostream>
#include <thread>

struct Help final: nil::cli::Command
{
    int run(const nil::cli::Options& options) const override
    {
        options.help(std::cout);
        return 0;
    }
};

template <typename T>
typename T::Options parse(const nil::cli::Options& options)
{
    const auto port = std::uint16_t(options.number("port"));
    if constexpr (std::is_same_v<nil::service::tcp::Server, T> || std::is_same_v<nil::service::udp::Server, T>)
    {
        return {.port = port};
    }
    else if constexpr (std::is_same_v<nil::service::tcp::Client, T> || std::is_same_v<nil::service::udp::Client, T>)
    {
        return {.host = "127.0.0.1", .port = port};
    }
    else
    {
        static_assert(
            std::is_same_v<nil::service::tcp::Server, T> || //
            std::is_same_v<nil::service::tcp::Client, T> || //
            std::is_same_v<nil::service::udp::Server, T> || //
            std::is_same_v<nil::service::udp::Client, T>    //
        );
    }
}

template <typename T>
struct Service: nil::cli::Command
{
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .number(
                "port",
                {
                    .skey = 'p',
                    .msg = "port",
                    .fallback = 8000,
                    .implicit = 8000 //
                }
            )
            .build();
    }

    int run(const nil::cli::Options& options) const override
    {
        static_assert(std::is_base_of_v<nil::service::IService, T>);
        const std::unique_ptr<nil::service::IService> service
            = std::make_unique<T>(parse<T>(options));
        service->on(
            nil::service::Event::Connect,
            [](std::uint32_t id) { //
                std::cout << "connected    : " << id << std::endl;
            }
        );
        service->on(
            nil::service::Event::Disconnect,
            [](std::uint32_t id) { //
                std::cout << "disconnected : " << id << std::endl;
            }
        );
        service->on(
            1,
            [](const void* data, std::uint64_t size)
            {
                const auto message = std::string_view(static_cast<const char*>(data), size);
                std::cout << "message      : " << message << std::endl;
            }
        );

        const std::thread t([&]() { service->start(); });
        std::string message;
        while (std::getline(std::cin, message))
        {
            service->publish(1, message.data(), message.size());
        }
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<Help>();
    {
        auto& tcp = root.add<Help>("tcp", "tcp");
        tcp.add<Service<nil::service::tcp::Server>>("server", "server");
        tcp.add<Service<nil::service::tcp::Client>>("client", "client");
    }
    {
        auto& udp = root.add<Help>("udp", "udp");
        udp.add<Service<nil::service::udp::Server>>("server", "server");
        udp.add<Service<nil::service::udp::Client>>("client", "client");
    }
    return root.run(argc, argv);
}
