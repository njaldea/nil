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
        T service(parse<T>(options));
        service.on_connect([](const std::string& id) { //
            std::cout << "connected    : " << id << std::endl;
        });
        service.on_disconnect([](const std::string& id) { //
            std::cout << "disconnected : " << id << std::endl;
        });
        service.on_message(
            [](const std::string& id, const void* data, std::uint64_t size)
            {
                const auto message = std::string_view(static_cast<const char*>(data), size);
                std::cout << "from         : " << id << std::endl;
                std::cout << "message      : " << message << std::endl;
            }
        );

        while (true)
        {
            std::thread t1([&]() { service.run(); });
            std::string message;
            while (std::getline(std::cin, message))
            {
                if (message == "reconnect")
                {
                    break;
                }
                service.publish(message.data(), message.size());
            }
            service.stop();
            t1.join();
            service.restart();
        }
        return 0;
    }
};

template <typename T>
void add_sub_nodes(nil::cli::Node& node)
{
    node.add<Service<typename T::Server>>("server", "server");
    node.add<Service<typename T::Client>>("client", "client");
}

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<Help>();
    add_sub_nodes<nil::service::udp::modes>(root.add<Help>("udp", "use udp protocol"));
    add_sub_nodes<nil::service::tcp::modes>(root.add<Help>("tcp", "use tcp protocol"));
    add_sub_nodes<nil::service::ws::modes>(root.add<Help>("ws", "use ws protocol"));
    return root.run(argc, argv);
}
