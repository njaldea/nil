#include <nil/service/tcp/Server.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <iostream>
#include <unordered_set>

namespace nil::service::tcp
{
    struct Connection: std::enable_shared_from_this<Connection>
    {
        Connection(boost::asio::io_context& context, Server::Impl& parent);
        ~Connection();

        void start();
        void readHeader(std::uint64_t pos, std::uint64_t size);
        void readBody(std::uint64_t pos, std::uint64_t size);
        void write(std::string message);

        boost::asio::ip::tcp::socket socket;
        Server::Impl& parent;

        std::vector<char> buffer;
    };

    struct Server::Impl
    {
        Impl(Server::Options options)
            : context()
            , endpoint(boost::asio::ip::make_address("0.0.0.0"), options.port)
            , acceptor(context, endpoint)
        {
        }

        boost::asio::io_context context;
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::ip::tcp::acceptor acceptor;
        std::unordered_set<Connection*> connections;

        void connect()
        {
            const auto connection = std::make_shared<Connection>(context, *this);
            acceptor.async_accept(
                connection->socket,
                [this, connection](const boost::system::error_code& ec)
                {
                    if (!ec)
                    {
                        connections.emplace(connection.get());
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

    Connection::Connection(boost::asio::io_context& context, Server::Impl& parent)
        : socket(context)
        , parent(parent)
    {
        buffer.reserve(1024);
    }

    Connection::~Connection()
    {
        if (parent.connections.contains(this))
        {
            parent.connections.erase(this);
        }
    }

    void Connection::start()
    {
        parent.connections.emplace(this);
        write("Hello from the Client");
        readHeader(0u, 8u);
    }

    void Connection::readHeader(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(buffer.data() + pos, size - pos),
            [pos, size, self = shared_from_this()]( //
                const boost::system::error_code& ec,
                size_t count
            )
            {
                if (ec)
                {
                    return;
                }

                if (pos + count != size)
                {
                    self->readHeader(pos + count, size);
                }
                else
                {
                    std::uint64_t esize = 0 //
                        | (std::uint64_t(self->buffer[0]) << (0 * 8))
                        | (std::uint64_t(self->buffer[1]) << (1 * 8))
                        | (std::uint64_t(self->buffer[2]) << (2 * 8))
                        | (std::uint64_t(self->buffer[3]) << (3 * 8))
                        | (std::uint64_t(self->buffer[4]) << (4 * 8))
                        | (std::uint64_t(self->buffer[5]) << (5 * 8))
                        | (std::uint64_t(self->buffer[6]) << (6 * 8))
                        | (std::uint64_t(self->buffer[7]) << (7 * 8));

                    self->readBody(0, esize);
                }
            }
        );
    }

    void Connection::readBody(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(buffer.data() + pos, size - pos),
            [pos, size, self = shared_from_this()]( //
                const boost::system::error_code& ec,
                size_t count
            )
            {
                if (ec)
                {
                    return;
                }

                if (pos + count != size)
                {
                    self->readBody(pos + count, size);
                }
                else
                {
                    std::cout << std::string(self->buffer.data(), count) << std::endl;
                    self->readHeader(0u, 8u);
                }
            }
        );
    }

    void Connection::write(std::string message)
    {
        const auto max_length = 1024;

        socket.async_write_some(
            boost::asio::buffer(message, max_length),
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
                // message sent.. what to do? normally nothing.
            }
        );
    }
}
