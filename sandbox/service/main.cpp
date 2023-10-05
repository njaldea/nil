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
    if constexpr (
        std::is_same_v<nil::service::tcp::Server, T> || //
        std::is_same_v<nil::service::udp::Server, T> || //
        std::is_same_v<nil::service::ws::Server, T>
    )
    {
        return {.port = port};
    }
    else if constexpr (
        std::is_same_v<nil::service::tcp::Client, T> || //
        std::is_same_v<nil::service::udp::Client, T> || //
        std::is_same_v<nil::service::ws::Client, T>
    )
    {
        return {.host = "127.0.0.1", .port = port};
    }
    else
    {
        static_assert(
            std::is_same_v<nil::service::tcp::Server, T> || //
            std::is_same_v<nil::service::tcp::Client, T> || //
            std::is_same_v<nil::service::udp::Server, T> || //
            std::is_same_v<nil::service::udp::Client, T> || //
            std::is_same_v<nil::service::ws::Server, T> ||  //
            std::is_same_v<nil::service::ws::Client, T>     //
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
        const auto service = std::make_unique<T>(parse<T>(options));
        service->on(
            nil::service::Event::Connect,
            [](const std::string& id) { //
                std::cout << "connected    : " << id << std::endl;
            }
        );
        service->on(
            nil::service::Event::Disconnect,
            [](const std::string& id) { //
                std::cout << "disconnected : " << id << std::endl;
            }
        );
        service->on(
            1,
            [](const void* data, std::uint64_t size)
            {
                const auto message = std::string_view(static_cast<const char*>(data), size);
                std::cout << "message      : " << message << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        );

        while (true)
        {
            std::thread t1([&]() { service->run(); });
            std::thread t2([&]() { service->run(); });
            std::thread t3([&]() { service->run(); });
            std::thread t4([&]() { service->run(); });
            std::string message;
            while (std::getline(std::cin, message))
            {
                if (message == "reconnect")
                {
                    break;
                }
                service->publish(1, message.data(), message.size());
            }
            service->stop();
            t1.join();
            t2.join();
            t3.join();
            t4.join();
            service->restart();
        }
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<Help>();
    {
        auto& udp = root.add<Help>("udp", "use udp protocol");
        udp.add<Service<nil::service::udp::Server>>("server", "server");
        udp.add<Service<nil::service::udp::Client>>("client", "client");
    }
    {
        auto& tcp = root.add<Help>("tcp", "use tcp protocol");
        tcp.add<Service<nil::service::tcp::Server>>("server", "server");
        tcp.add<Service<nil::service::tcp::Client>>("client", "client");
    }
    {
        auto& ws = root.add<Help>("ws", "use ws protocol");
        ws.add<Service<nil::service::ws::Server>>("server", "server");
        ws.add<Service<nil::service::ws::Client>>("client", "client");
    }
    return root.run(argc, argv);
}
