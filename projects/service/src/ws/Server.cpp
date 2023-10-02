#include <nil/service/ws/Server.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace nil::service::ws
{
    struct Server::Impl final: IImpl
    {
        explicit Impl(Server::Options options)
            : options(options)
            , endpoint(boost::asio::ip::make_address("0.0.0.0"), this->options.port)
            , acceptor(context, endpoint)
        {
        }

        Server::Options options;

        struct Handlers
        {
            std::unordered_map<std::uint32_t, MsgHandler> msg;
            EventHandler connect;
            EventHandler disconnect;
        } handlers;

        boost::asio::io_context context;
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::ip::tcp::acceptor acceptor;
        std::unordered_map<std::uint16_t, Connection*> connections;

        void send(
            std::uint16_t id,
            std::uint32_t type,
            const std::uint8_t* data,
            std::uint64_t size
        )
        {
            context.dispatch(
                [this, id, type, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    const auto it = connections.find(id);
                    if (it != connections.end())
                    {
                        it->second->write(type, msg.data(), msg.size());
                    }
                }
            );
        }

        void publish(std::uint32_t type, const std::uint8_t* data, std::uint64_t size)
        {
            context.dispatch(
                [this, type, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    for (const auto& item : connections)
                    {
                        item.second->write(type, msg.data(), msg.size());
                    }
                }
            );
        }

        void connect(Connection* connection) override
        {
            const auto port = connection->handle().remote_endpoint().port();
            if (!connections.contains(port))
            {
                connections.emplace(port, connection);
            }
            if (handlers.connect)
            {
                handlers.connect(port);
            }
        }

        void disconnect(Connection* connection) override
        {
            const auto port = connection->handle().remote_endpoint().port();
            if (connections.contains(port))
            {
                connections.erase(port);
            }
            if (handlers.disconnect)
            {
                handlers.disconnect(port);
            }
        }

        void message(std::uint32_t type, const std::uint8_t* data, std::uint64_t size) override
        {
            const auto it = handlers.msg.find(type);
            if (it != handlers.msg.end() && it->second)
            {
                it->second(data, size);
            }
        }

        void start()
        {
            auto conn = std::make_shared<Connection>(options.buffer, context, *this);
            acceptor.async_accept(
                conn->handle(),
                [this, conn](const boost::system::error_code& ec)
                {
                    if (!ec)
                    {
                        conn->connected();
                    }
                    start();
                }
            );
        }
    };

    Server::Server(Server::Options options)
        : impl(std::make_unique<Impl>(options))
    {
    }

    Server::~Server() noexcept = default;

    void Server::start()
    {
        impl->start();
        impl->context.run();
    }

    void Server::stop()
    {
        impl->context.stop();
    }

    void Server::on(std::uint32_t type, MsgHandler handler)
    {
        impl->handlers.msg.emplace(type, std::move(handler));
    }

    void Server::on(Event event, EventHandler handler)
    {
        switch (event)
        {
            case Event::Connect:
                impl->handlers.connect = std::move(handler);
                break;
            case Event::Disconnect:
                impl->handlers.disconnect = std::move(handler);
                break;
            default:
                throw std::runtime_error("unknown type");
        }
    }

    void Server::send(std::uint16_t id, std::uint32_t type, const void* data, std::uint64_t size)
    {
        impl->send(id, type, static_cast<const std::uint8_t*>(data), size);
    }

    void Server::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        impl->publish(type, static_cast<const std::uint8_t*>(data), size);
    }
}
