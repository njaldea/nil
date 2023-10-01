#include <nil/service/tcp/Client.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>

namespace nil::service::tcp
{
    struct Client::Impl final: IImpl
    {
        explicit Impl(Client::Options options)
            : options(std::move(options))
            , endpoint(boost::asio::ip::make_address(this->options.host.data()), this->options.port)
            , reconnection(context)
        {
        }

        void publish(std::uint32_t type, const std::uint8_t* data, std::uint64_t size)
        {
            context.dispatch(
                [this, type, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    if (connection != nullptr)
                    {
                        connection->write(type, msg.data(), msg.size());
                    }
                }
            );
        }

        void connect(Connection* connection) override
        {
            (void)connection;
            if (handlers.connect)
            {
                handlers.connect(options.port);
            }
        }

        void disconnect(Connection* connection) override
        {
            if (this->connection == connection)
            {
                this->connection = nullptr;
                if (handlers.disconnect)
                {
                    handlers.disconnect(options.port);
                }
            }
            reconnect();
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
        : impl(std::make_unique<Impl>(std::move(options)))

    {
    }

    Client::~Client() noexcept = default;

    void Client::start()
    {
        impl->start();
        impl->context.run();
    }

    void Client::stop()
    {
        impl->context.stop();
    }

    void Client::on(std::uint32_t type, MsgHandler handler)
    {
        impl->handlers.msg.emplace(type, std::move(handler));
    }

    void Client::on(Event event, EventHandler handler)
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

    void Client::send(std::uint16_t id, std::uint32_t type, const void* data, std::uint64_t size)
    {
        if (id != impl->options.port)
        {
            impl->publish(type, static_cast<const std::uint8_t*>(data), size);
        }
    }

    void Client::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        impl->publish(type, static_cast<const std::uint8_t*>(data), size);
    }
}
