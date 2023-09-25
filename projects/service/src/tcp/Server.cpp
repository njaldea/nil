#include <nil/service/tcp/Server.hpp>

#include "Connection.hpp"
#include <nil_service_message.pb.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <unordered_set>

namespace nil::service::tcp
{
    struct Server::Impl final: IImpl
    {
        Impl(Server::Options options)
            : options(std::move(options))
            , context()
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
            const void* data,
            std::uint64_t size //
        )
        {
            Message msg;
            msg.set_internal(false);
            msg.set_type(type);
            msg.set_data(data, size);
            context.dispatch(
                [this, id, msg = msg.SerializeAsString()]()
                {
                    const auto it = connections.find(id);
                    if (it != connections.end())
                    {
                        it->second->write(msg.data(), msg.size());
                    }
                }
            );
        }

        void publish(std::uint32_t type, const void* data, std::uint64_t size)
        {
            Message msg;
            msg.set_internal(false);
            msg.set_type(type);
            msg.set_data(data, size);
            context.dispatch(
                [this, msg = msg.SerializeAsString()]()
                {
                    for (const auto& item : connections)
                    {
                        item.second->write(msg.data(), msg.size());
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

        void message(const void* data, std::uint64_t size) override
        {
            Message message;
            message.ParseFromArray(data, int(size));
            if (!message.internal())
            {
                const auto it = handlers.msg.find(message.type());
                if (it != handlers.msg.end())
                {
                    if (it->second)
                    {
                        const auto& inner = message.data();
                        it->second(inner.data(), inner.size());
                    }
                }
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
        : mImpl(std::make_unique<Impl>(std::move(options)))
    {
    }

    Server::~Server() noexcept = default;

    void Server::on(std::uint32_t type, MsgHandler handler)
    {
        mImpl->handlers.msg.emplace(type, std::move(handler));
    }

    void Server::on(Event event, EventHandler handler)
    {
        switch (event)
        {
            case Event::Connect:
                mImpl->handlers.connect = std::move(handler);
                break;
            case Event::Disconnect:
                mImpl->handlers.disconnect = std::move(handler);
                break;
            default:
                throw std::runtime_error("unknown type");
        }
    }

    void Server::start()
    {
        mImpl->start();
        mImpl->context.run();
    }

    void Server::stop()
    {
        mImpl->context.stop();
    }

    void Server::send(
        std::uint16_t id,
        std::uint32_t type,
        const void* data,
        std::uint64_t size //
    )
    {
        mImpl->send(id, type, data, size);
    }

    void Server::publish(
        std::uint32_t type,
        const void* data,
        std::uint64_t size //
    )
    {
        mImpl->publish(type, data, size);
    }
}
