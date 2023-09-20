#include <nil/service/tcp/Client.hpp>

#include "Connection.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <iostream>

namespace nil::service::tcp
{
    struct Client::Impl
    {
        Impl(Client::Options options)
            : context()
            , endpoint(boost::asio::ip::make_address(options.host.data()), options.port)
            , reconnection(context)
        {
        }

        void connect()
        {
            reconnection.cancel();
            auto connection = std::make_shared<Connection>(context, handlers);
            connection->handle().async_connect(
                endpoint,
                [this, connection](const boost::system::error_code& ec)
                {
                    if (ec)
                    {
                        this->reconnect();
                        return;
                    }
                    this->connection = connection;
                    connection->start();
                }
            );
        }

        void reconnect()
        {
            connection.reset();
            reconnection.expires_from_now(boost::posix_time::milliseconds(25));
            reconnection.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        this->connect();
                    }
                }
            );
        }

        std::unordered_map<int, std::unique_ptr<IHandler>> handlers;

        boost::asio::io_context context;

        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::deadline_timer reconnection;
        std::shared_ptr<Connection> connection;
    };

    Client::Client(Client::Options options)
        : mImpl(std::make_unique<Impl>(std::move(options)))
    {
    }

    Client::~Client() noexcept = default;

    void Client::on(int type, std::unique_ptr<IHandler> handler)
    {
        mImpl->handlers.emplace(type, std::move(handler));
    }

    void Client::start()
    {
        mImpl->connect();
        mImpl->context.run();
    }

    void Client::stop()
    {
        mImpl->context.stop();
    }

    void Client::publish(int type, std::string msg)
    {
        (void)type;
        mImpl->context.dispatch(
            [connection = mImpl->connection, msg = std::move(msg)]()
            {
                if (connection)
                {
                    connection->write(msg);
                }
            }
        );
    }
}
