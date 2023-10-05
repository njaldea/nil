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
        explicit Impl(const detail::Storage<Options>& storage)
            : storage(storage)
            , strand(boost::asio::make_strand(context))
            , endpoint(
                  boost::asio::ip::make_address(storage.options.host.data()),
                  storage.options.port
              )
            , reconnection(strand)
        {
        }

        void publish(std::uint32_t type, const std::uint8_t* data, std::uint64_t size)
        {
            boost::asio::dispatch(
                strand,
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
            boost::asio::dispatch(
                strand,
                [this, connection]()
                {
                    this->connection = connection;
                    if (storage.connect)
                    {
                        storage.connect(storage.options.port);
                    }
                }
            );
        }

        void disconnect(Connection* connection) override
        {
            boost::asio::dispatch(
                strand,
                [this, connection]()
                {
                    if (this->connection == connection)
                    {
                        this->connection = nullptr;
                        if (storage.disconnect)
                        {
                            storage.disconnect(storage.options.port);
                        }
                    }
                    reconnect();
                }
            );
        }

        void message(std::uint32_t type, const std::uint8_t* data, std::uint64_t size) override
        {
            const auto it = storage.msg.find(type);
            if (it != storage.msg.end() && it->second)
            {
                it->second(data, size);
            }
        }

        void connect()
        {
            auto socket = std::make_unique<boost::asio::ip::tcp::socket>(strand);
            auto* socket_ptr = socket.get();
            socket_ptr->async_connect(
                endpoint,
                [this, socket = std::move(socket)](const boost::system::error_code& ec)
                {
                    if (!ec)
                    {
                        std::make_shared<Connection>(
                            storage.options.buffer,
                            std::move(*socket),
                            *this
                        )
                            ->start();
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
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::steady_timer reconnection;
        Connection* connection = nullptr;
    };

    Client::Client(Client::Options options)
        : storage{std::move(options)}
        , impl()
    {
    }

    Client::~Client() noexcept = default;

    void Client::prepare()
    {
        impl.reset();
        impl = std::make_unique<Impl>(storage);
        impl->connect();
    }

    void Client::run()
    {
        if (impl)
        {
            impl->context.run();
        }
    }

    void Client::stop()
    {
        if (impl)
        {
            impl->context.stop();
        }
    }

    void Client::on(std::uint32_t type, MsgHandler handler)
    {
        storage.msg.emplace(type, std::move(handler));
    }

    void Client::on(Event event, EventHandler handler)
    {
        switch (event)
        {
            case Event::Connect:
                storage.connect = std::move(handler);
                break;
            case Event::Disconnect:
                storage.disconnect = std::move(handler);
                break;
            default:
                throw std::runtime_error("unknown type");
        }
    }

    void Client::send(std::uint16_t id, std::uint32_t type, const void* data, std::uint64_t size)
    {
        if (impl && id != storage.options.port)
        {
            impl->publish(type, static_cast<const std::uint8_t*>(data), size);
        }
    }

    void Client::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        if (impl)
        {
            impl->publish(type, static_cast<const std::uint8_t*>(data), size);
        }
    }
}
