#include <nil/service/tcp/Client.hpp>

#include "Connection.hpp"
#include <nil_service_message.pb.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

namespace nil::service::tcp
{
    struct Client::Impl final: IImpl
    {
        Impl(Client::Options options)
            : options(std::move(options))
            , context()
            , endpoint(boost::asio::ip::make_address(this->options.host.data()), this->options.port)
            , reconnection(context)
        {
        }

        void publish(std::uint32_t type, const void* data, std::uint64_t size)
        {
            Message msg;
            msg.set_type(type);
            msg.set_data(data, size);
            context.dispatch(
                [this, msg = msg.SerializeAsString()]()
                {
                    if (connection)
                    {
                        connection->write(msg.data(), msg.size());
                    }
                }
            );
        }

        void connect(Connection* connection) override
        {
            (void)connection;
            if (handlers.connect)
            {
                handlers.connect();
            }
        }

        void disconnect(Connection* connection) override
        {
            if (this->connection == connection)
            {
                this->connection = nullptr;
                if (handlers.disconnect)
                {
                    handlers.disconnect();
                }
            }
            reconnect();
        }

        void message(const void* data, std::uint64_t size) override
        {
            Message message;
            message.ParseFromArray(data, int(size));
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

        void start()
        {
            reconnection.cancel();
            auto conn = std::make_shared<Connection>(options.buffer, context, *this);
            conn->handle().async_connect(
                endpoint,
                [this, conn](const boost::system::error_code& ec)
                {
                    if (ec)
                    {
                        reconnect();
                        return;
                    }
                    connection = conn.get();
                    connection->connected();
                }
            );
        }

        void reconnect()
        {
            reconnection.expires_from_now(std::chrono::milliseconds(25));
            reconnection.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        start();
                    }
                }
            );
        }

        Client::Options options;

        struct Handlers
        {
            std::unordered_map<std::uint32_t, MsgHandler> msg;
            EventHandler connect;
            EventHandler disconnect;
        } handlers;

        boost::asio::io_context context;
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::steady_timer reconnection;
        Connection* connection = nullptr;
    };

    Client::Client(Client::Options options)
        : mImpl(std::make_unique<Impl>(std::move(options)))

    {
    }

    Client::~Client() noexcept = default;

    void Client::on(std::uint32_t type, MsgHandler handler)
    {
        mImpl->handlers.msg.emplace(type, std::move(handler));
    }

    void Client::on(Event event, EventHandler handler)
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

    void Client::start()
    {
        mImpl->start();
        mImpl->context.run();
    }

    void Client::stop()
    {
        mImpl->context.stop();
    }

    void Client::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        mImpl->publish(type, data, size);
    }
}
