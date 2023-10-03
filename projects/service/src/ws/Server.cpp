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
            , acceptor(context, endpoint, true)
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
            const auto id = connection->id();
            if (!connections.contains(id))
            {
                connections.emplace(id, connection);
            }
            if (handlers.connect)
            {
                handlers.connect(id);
            }
        }

        void disconnect(Connection* connection) override
        {
            const auto id = connection->id();
            if (connections.contains(id))
            {
                connections.erase(id);
            }
            if (handlers.disconnect)
            {
                handlers.disconnect(id);
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
            acceptor.async_accept(
                [this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket)
                {
                    if (!ec)
                    {
                        namespace websocket = boost::beast::websocket;
                        auto ws = std::make_unique<websocket::stream<boost::beast::tcp_stream>>(
                            std::move(socket)
                        );
                        ws->set_option(websocket::stream_base::timeout::suggested(
                            boost::beast::role_type::server
                        ));
                        ws->set_option(websocket::stream_base::decorator(
                            [](websocket::response_type& res)
                            {
                                res.set(
                                    boost::beast::http::field::server,
                                    std::string(BOOST_BEAST_VERSION_STRING)
                                        + " websocket-server-async"
                                );
                            }
                        ));
                        auto* ws_ptr = ws.get();
                        ws_ptr->async_accept(
                            [this, ws = std::move(ws)](boost::beast::error_code ec)
                            {
                                if (ec)
                                {
                                    return;
                                }
                                std::make_shared<Connection>(options.buffer, std::move(*ws), *this)
                                    ->start();
                            }
                        );
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
