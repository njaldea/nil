#include <nil/service/tcp/Server.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <iostream>

namespace nil::service::tcp
{
    struct Connection: std::enable_shared_from_this<Connection>
    {
        Connection(boost::asio::io_context& context)
            : socket(context)
        {
        }

        ~Connection()
        {
        }

        void start()
        {
            write();
            read();
        }

        void read()
        {
            const auto max_length = 1024;
            static char data[max_length];

            socket.async_read_some(
                boost::asio::buffer(data, max_length),
                [self = shared_from_this()]( //
                    const boost::system::error_code& ec,
                    size_t count
                )
                {
                    if (ec)
                    {
                        return;
                    }
                    (void)count;
                    // data should have been populated;
                    self->read();
                }
            );
        }

        void write()
        {
            const auto max_length = 1024;

            socket.async_write_some(
                boost::asio::buffer("Hello From Server! You are connected!", max_length),
                [self = shared_from_this()]( //
                    const boost::system::error_code& ec,
                    size_t count
                )
                {
                    (void)ec;
                    (void)count;
                    // message sent.. what to do? normally nothing.
                }
            );
        }

        boost::asio::ip::tcp::socket socket;
    };

    struct Server::Impl
    {
        Impl(std::string_view host, std::uint16_t port)
            : context()
            , endpoint(boost::asio::ip::make_address(host.data()), port)
            , acceptor(context, endpoint)
        {
        }

        boost::asio::io_context context;
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::ip::tcp::acceptor acceptor;

        void connect()
        {
            const auto connection = std::make_shared<Connection>(context);
            acceptor.async_accept(
                connection->socket,
                [this, connection](const boost::system::error_code& err)
                {
                    if (!err)
                    {
                        connection->start();
                    }
                    this->connect();
                }
            );
        }
    };

    Server::Server(std::string host, int port)
        : mImpl(std::make_unique<Impl>(std::move(host), port))
    {
    }

    Server::~Server() noexcept = default;

    void Server::on(int type, std::unique_ptr<IHandler> handler)
    {
        (void)type;
        (void)handler;
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
}
