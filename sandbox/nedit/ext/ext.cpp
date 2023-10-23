#include "ext.hpp"

#include <nil/service.hpp>

#include <iostream>
#include <thread>

nil::cli::OptionInfo EXT::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "port"})
        .build();
}

int EXT::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }
    nil::service::tcp::Client client({
        .host = "127.0.0.1",                          //
        .port = std::uint16_t(options.number("port")) //
    });
    client.on_connect(
        [&](const std::string& id)
        {
            client.send(id, "pin:1,0.5,0.5,1", sizeof("pin:1,0.5,0.5,1"));
            client.send(id, "pin:0.5,1,0.5,1", sizeof("pin:0.5,1,0.5,1"));
            client.send(id, "node:0-0", sizeof("node:0-0"));
            client.send(id, "node:0-1", sizeof("node:0-1"));
            client.send(id, "node:1-1", sizeof("node:1-1"));
            client.send(id, "node:1-0", sizeof("node:1-0"));
            client.send(id, "node:0,1-1,0", sizeof("node:0,1-1,0"));
        }
    );

    while (true)
    {
        std::thread t1([&]() { client.run(); });
        std::string message;
        while (std::getline(std::cin, message))
        {
            if (message == "reconnect")
            {
                break;
            }
            client.publish(message.data(), message.size());
        }
        client.stop();
        t1.join();
        client.restart();
    }
    return 0;
}
