#include <nil/cli.hpp>
#include <nil/service.hpp>

#include <iostream>
#include <thread>

struct Handler: nil::service::IHandler
{
    void exec(const std::string& message) override
    {
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << message << std::endl;
    }
};

struct Root: nil::cli::Command
{
    int run(const nil::cli::Options& options) const
    {
        options.help(std::cout);
        return 0;
    }
};

struct Server: nil::cli::Command
{
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder() //
            .number("port", {.skey = 'p', .msg = "port", .fallback = 8000, .implicit = 8000})
            .build();
    }

    int run(const nil::cli::Options& options) const
    {
        nil::service::tcp::Server server({.port = std::uint16_t(options.number("port"))});
        server.on(0, std::make_unique<Handler>());
        server.start();
        return 0;
    }
};

struct Client: nil::cli::Command
{
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder() //
            .number("port", {.skey = 'p', .msg = "port", .fallback = 8000, .implicit = 8000})
            .build();
    }

    int run(const nil::cli::Options& options) const
    {
        nil::service::tcp::Client client(
            {.host = "127.0.0.1", .port = std::uint16_t(options.number("port"))}
        );

        std::thread t([&]() { client.start(); });
        std::string message;
        while (std::getline(std::cin, message))
        {
            client.publish(1, std::move(message));
        }
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<Root>();
    root.add<Server>("server", "server");
    root.add<Client>("client", "client");
    return root.run(argc, argv);
}
