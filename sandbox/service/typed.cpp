#include <nil/cli.hpp>
#include <nil/cli/nodes/Help.hpp>
#include <nil/service.hpp>

#include <iostream>
#include <thread>

template <typename Service>
std::unique_ptr<nil::service::IService> make_service(const nil::cli::Options& options)
{
    const auto port = std::uint16_t(options.number("port"));
    if constexpr (
        std::is_same_v<nil::service::tcp::Server, Service> || //
        std::is_same_v<nil::service::udp::Server, Service> || //
        std::is_same_v<nil::service::ws::Server, Service>
    )
    {
        return nil::service::make_service<Service>({.port = port});
    }
    else if constexpr (
        std::is_same_v<nil::service::tcp::Client, Service> || //
        std::is_same_v<nil::service::udp::Client, Service> || //
        std::is_same_v<nil::service::ws::Client, Service>
    )
    {
        return nil::service::make_service<Service>({.host = "127.0.0.1", .port = port});
    }
    else
    {
        static_assert(
            std::is_same_v<nil::service::tcp::Server, Service> || //
            std::is_same_v<nil::service::tcp::Client, Service> || //
            std::is_same_v<nil::service::udp::Server, Service> || //
            std::is_same_v<nil::service::udp::Client, Service> || //
            std::is_same_v<nil::service::ws::Server, Service> ||  //
            std::is_same_v<nil::service::ws::Client, Service>     //
        );
    }
}

template <typename T>
struct Service final: nil::cli::Command
{
    static_assert(std::is_base_of_v<nil::service::IService, T>);

    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .flag("help", {.skey = 'h', .msg = "this help"})
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

    void add_handlers(nil::service::TypedService& service) const
    {
        service.on_message(
            0,
            [](const std::string&, const std::string& message)
            {
                std::cout << "from         : hidden" << std::endl;
                std::cout << "type         : " << 0 << std::endl;
                std::cout << "message      : " << message << std::endl;
            }
        );
        service.on_message(
            1,
            [](const std::string& id, const std::string& message)
            {
                std::cout << "from         : " << id << std::endl;
                std::cout << "type         : " << 1 << std::endl;
                std::cout << "message      : " << message << std::endl;
            }
        );
        service.on_connect(             //
            [](const std::string& id) { //
                std::cout << "connected    : " << id << std::endl;
            }
        );
        service.on_disconnect(          //
            [](const std::string& id) { //
                std::cout << "disconnected : " << id << std::endl;
            }
        );
    }

    void loop(nil::service::TypedService& service) const
    {
        while (true)
        {
            std::thread t1([&]() { service.run(); });
            std::string message;
            std::uint32_t type = 0;
            while (std::getline(std::cin, message))
            {
                if (message == "reconnect")
                {
                    break;
                }
                service.publish(type, message);
                type = (type + 1) % 2;
            }
            service.stop();
            t1.join();
            service.restart();
        }
    }

    int run(const nil::cli::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        nil::service::TypedService service(make_service<T>(options));
        add_handlers(service);
        loop(service);
        return 0;
    }
};

template <typename Server, typename Client>
void add_sub_nodes(nil::cli::Node& node)
{
    node.add<Service<Server>>("server", "server");
    node.add<Service<Client>>("client", "client");
}

int main(int argc, const char** argv)
{
    using nil::cli::Node;
    using nil::cli::nodes::Help;
    using namespace nil::service;

    auto root = Node::root<Help>(std::cout);
    add_sub_nodes<udp::Server, udp::Client>(root.add<Help>("udp", "use udp protocol", std::cout));
    add_sub_nodes<tcp::Server, tcp::Client>(root.add<Help>("tcp", "use tcp protocol", std::cout));
    add_sub_nodes<ws::Server, ws::Client>(root.add<Help>("ws", "use ws protocol", std::cout));
    return root.run(argc, argv);
}
