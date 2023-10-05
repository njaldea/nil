#include <nil/service/ws/Client.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::ws
{
    struct Client::Impl final: IImpl
    {
        explicit Impl(const detail::Storage<Options>& storage)
            : storage(storage)
            , strand(boost::asio::make_strand(context))
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
                        storage.connect(connection->id());
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
                            storage.disconnect(connection->id());
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
                {boost::asio::ip::make_address(storage.options.host.data()), storage.options.port},
                [this, socket = std::move(socket)](const boost::system::error_code& ec)
                {
                    if (ec)
                    {
                        reconnect();
                        return;
                    }
                    namespace websocket = boost::beast::websocket;
                    auto ws = std::make_unique<websocket::stream<boost::beast::tcp_stream>>(
                        std::move(*socket)
                    );
                    ws->set_option(
                        websocket::stream_base::timeout::suggested(boost::beast::role_type::client)
                    );
                    ws->set_option(websocket::stream_base::decorator(
                        [](websocket::request_type& req)
                        {
                            req.set(
                                boost::beast::http::field::user_agent,
                                std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async"
                            );
                        }
                    ));
                    auto* ws_ptr = ws.get();
                    ws_ptr->async_handshake(
                        storage.options.host + ':' + std::to_string(storage.options.port),
                        "/",
                        [this, ws = std::move(ws)](boost::beast::error_code ec)
                        {
                            if (ec)
                            {
                                reconnect();
                                return;
                            }
                            std::make_shared<Connection>(
                                storage.options.buffer,
                                std::move(*ws),
                                *this
                            )
                                ->start();
                        }
                    );
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
        Connection* connection = nullptr;
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

    void Client::send(
        const std::string& id,
        std::uint32_t type,
        const void* data,
        std::uint64_t size
    )
    {
        (void)id;
        impl->publish(type, static_cast<const std::uint8_t*>(data), size);
    }

    void Client::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        impl->publish(type, static_cast<const std::uint8_t*>(data), size);
    }
}
