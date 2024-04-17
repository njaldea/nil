#include <nil/clix.hpp>
#include <nil/clix/nodes/Help.hpp>
#include <nil/service.hpp>

#include <iostream>
#include <thread>

template <typename Service>
std::unique_ptr<nil::service::IService> make_service(const nil::clix::Options& options)
{
    const auto port = std::uint16_t(options.number("port"));
    constexpr auto is_server //
        = std::is_same_v<nil::service::tcp::Server, Service>
        || std::is_same_v<nil::service::udp::Server, Service>
        || std::is_same_v<nil::service::ws::Server, Service>;
    constexpr auto is_client //
        = std::is_same_v<nil::service::tcp::Client, Service>
        || std::is_same_v<nil::service::udp::Client, Service>
        || std::is_same_v<nil::service::ws::Client, Service>;
    if constexpr (is_server)
    {
        return nil::service::make_service<Service>({.port = port});
    }
    else if constexpr (is_client)
    {
        return nil::service::make_service<Service>({.host = "127.0.0.1", .port = port});
    }
    else
    {
        static_assert(
            std::is_same_v<nil::service::tcp::Server, Service>    //
            || std::is_same_v<nil::service::tcp::Client, Service> //
            || std::is_same_v<nil::service::udp::Server, Service> //
            || std::is_same_v<nil::service::udp::Client, Service> //
            || std::is_same_v<nil::service::ws::Server, Service>  //
            || std::is_same_v<nil::service::ws::Client, Service> 
        );
    }
}

template <typename T>
struct Service final: nil::clix::Command
{
    static_assert(std::is_base_of_v<nil::service::IService, T>);

    nil::clix::OptionInfo options() const override
    {
        return nil::clix::Builder()
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

    void add_handlers(nil::service::IService& service) const
    {
        service.on_message(                             //
            nil::service::TypedHandler<std::uint32_t>() //
                .add(
                    0u,
                    [](const std::string& id, const std::string& message)
                    {
                        std::cout << "from         : " << id << std::endl;
                        std::cout << "type         : " << 0 << std::endl;
                        std::cout << "message      : " << message << std::endl;
                    }
                )
                .add(
                    1u,
                    [](const std::string& id, const std::string& message)
                    {
                        std::cout << "from         : " << id << std::endl;
                        std::cout << "type         : " << 1 << std::endl;
                        std::cout << "message      : " << message << std::endl;
                    }
                )
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

    void loop(nil::service::IService& service) const
    {
        using namespace std::string_literals;
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

                service.publish(type, "typed > " + message, " : "s, "secondary here"s);

                type = (type + 1) % 2;
            }
            service.stop();
            t1.join();
            service.restart();
        }
    }

    int run(const nil::clix::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        const auto service = make_service<T>(options);
        add_handlers(*service);
        loop(*service);
        return 0;
    }
};

template <typename Server, typename Client>
void add_sub_nodes(nil::clix::Node& node)
{
    node.add<Service<Server>>("server", "server");
    node.add<Service<Client>>("client", "client");
}

int main(int argc, const char** argv)
{
    using nil::clix::Node;
    using nil::clix::nodes::Help;
    using namespace nil::service;

    auto root = Node::root<Help>(std::cout);
    add_sub_nodes<udp::Server, udp::Client>(root.add<Help>("udp", "use udp protocol", std::cout));
    add_sub_nodes<tcp::Server, tcp::Client>(root.add<Help>("tcp", "use tcp protocol", std::cout));
    add_sub_nodes<ws::Server, ws::Client>(root.add<Help>("ws", "use ws protocol", std::cout));
    return root.run(argc, argv);
}
