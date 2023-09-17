#include <nil/cli.hpp>
#include <nil/service.hpp>

#include <iostream>

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
        nil::service::tcp::Server server("127.0.0.1", options.number("port"));
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
        nil::service::tcp::Client client("127.0.0.1", options.number("port"));
        client.start();
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
