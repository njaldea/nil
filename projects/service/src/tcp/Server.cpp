#include <nil/service/tcp/Server.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <unordered_set>

namespace nil::service::tcp
{
    struct Server::Impl
    {
        Impl(Server::Options options)
            : options(std::move(options))
            , context()
            , endpoint(boost::asio::ip::make_address("0.0.0.0"), options.port)
            , acceptor(context, endpoint)
        {
        }

        Server::Options options;
        std::unordered_map<int, std::unique_ptr<IHandler>> handlers;

        boost::asio::io_context context;
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::ip::tcp::acceptor acceptor;
        std::unordered_set<Connection*> connections;

        void connect()
        {
            auto connection = std::make_shared<Connection>( //
                options.buffer,
                context,
                handlers,
                &connections
            );
            acceptor.async_accept(
                connection->handle(),
                [this, connection](const boost::system::error_code& ec)
                {
                    if (!ec)
                    {
                        connection->start();
                    }
                    this->connect();
                }
            );
        }
    };

    Server::Server(Server::Options options)
        : mImpl(std::make_unique<Impl>(std::move(options)))
    {
    }

    Server::~Server() noexcept = default;

    void Server::on(int type, std::unique_ptr<IHandler> handler)
    {
        mImpl->handlers.emplace(type, std::move(handler));
    }

    void Server::start()
    {
        mImpl->connect();
        mImpl->context.run();
    }

    void Server::stop()
    {
        mImpl->context.stop();
    }

    void Server::publish(int type, std::string msg)
    {
        (void)type;
        mImpl->context.dispatch(
            [&, msg = std::move(msg)]()
            {
                for (auto* c : mImpl->connections)
                {
                    c->write(msg);
                }
            }
        );
    }
}
