#include <nil/service/tcp/Client.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::tcp
{
    struct Client::Impl final: IImpl
    {
        explicit Impl(const detail::Storage<Options>& init_storage)
            : storage(init_storage)
            , strand(boost::asio::make_strand(context))
            , reconnection(strand)
        {
        }

        void send(const std::string& id, const std::uint8_t* data, std::uint64_t size)
        {
            boost::asio::dispatch(
                strand,
                [this, id, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    if (connection != nullptr && connection->id() == id)
                    {
                        connection->write(msg.data(), msg.size());
                    }
                }
            );
        }

        void publish(const std::uint8_t* data, std::uint64_t size)
        {
            boost::asio::dispatch(
                strand,
                [this, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    if (connection != nullptr)
                    {
                        connection->write(msg.data(), msg.size());
                    }
                }
            );
        }

        void disconnect(Connection* target_connection) override
        {
            boost::asio::dispatch(
                strand,
                [this, target_connection]()
                {
                    if (connection.get() == target_connection)
                    {
                        if (storage.disconnect)
                        {
                            storage.disconnect(connection->id());
                        }
                        connection.reset();
                    }
                    reconnect();
                }
            );
        }

        void message(const std::string& id, const std::uint8_t* data, std::uint64_t size) override
        {
            if (storage.msg)
            {
                storage.msg(id, data, size);
            }
        }

        void connect()
        {
            auto socket = std::make_unique<boost::asio::ip::tcp::socket>(strand);
            auto* socket_ptr = socket.get();
            socket_ptr->async_connect(
                {boost::asio::ip::make_address(storage.options.host.data()), storage.options.port},
                [this, socket = std::move(socket)](const boost::system::error_code& ec)
                {
                    if (!ec)
                    {
                        connection = std::make_unique<Connection>(
                            storage.options.buffer,
                            std::move(*socket),
                            *this
                        );
                        if (storage.connect)
                        {
                            storage.connect(connection->id());
                        }
                        return;
                    }
                    reconnect();
                }
            );
        }

        void reconnect()
        {
            reconnection.expires_after(std::chrono::milliseconds(25));
            reconnection.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        connect();
                    }
                }
            );
        }

        const detail::Storage<Options>& storage;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::steady_timer reconnection;
        std::unique_ptr<Connection> connection;
    };

    Client::Client(Client::Options options)
        : storage{std::move(options)}
        , impl(std::make_unique<Impl>(storage))
    {
        impl->connect();
    }

    Client::~Client() noexcept = default;

    void Client::run()
    {
        impl->context.run();
    }

    void Client::stop()
    {
        impl->context.stop();
    }

    void Client::restart()
    {
        impl.reset();
        impl = std::make_unique<Impl>(storage);
        impl->connect();
    }

    void Client::on_message(MessageHandler handler)
    {
        storage.msg = std::move(handler);
    }

    void Client::on_connect(ConnectHandler handler)
    {
        storage.connect = std::move(handler);
    }

    void Client::on_disconnect(DisconnectHandler handler)
    {
        storage.disconnect = std::move(handler);
    }

    void Client::send(const std::string& id, const void* data, std::uint64_t size)
    {
        impl->send(id, static_cast<const std::uint8_t*>(data), size);
    }

    void Client::publish(const void* data, std::uint64_t size)
    {
        impl->publish(static_cast<const std::uint8_t*>(data), size);
    }
}
