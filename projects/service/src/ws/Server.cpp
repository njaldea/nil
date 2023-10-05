#include <nil/service/ws/Server.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::ws
{
    struct Server::Impl final: IImpl
    {
        explicit Impl(const detail::Storage<Options>& storage)
            : storage(storage)
            , strand(boost::asio::make_strand(context))
            , endpoint(boost::asio::ip::make_address("0.0.0.0"), storage.options.port)
            , acceptor(strand, endpoint, true)
        {
        }

        void send(
            const std::string& id,
            std::uint32_t type,
            const std::uint8_t* data,
            std::uint64_t size
        )
        {
            boost::asio::dispatch(
                strand,
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
            boost::asio::dispatch(
                strand,
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
            boost::asio::dispatch(
                strand,
                [this, connection]()
                {
                    const auto id = connection->id();
                    if (!connections.contains(id))
                    {
                        connections.emplace(id, connection);
                    }
                    if (storage.connect)
                    {
                        storage.connect(id);
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
                    const auto id = connection->id();
                    if (connections.contains(id))
                    {
                        connections.erase(id);
                    }
                    if (storage.disconnect)
                    {
                        storage.disconnect(id);
                    }
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

        void accept()
        {
            acceptor.async_accept(
                boost::asio::make_strand(context),
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
                                std::make_shared<Connection>(
                                    storage.options.buffer,
                                    std::move(*ws),
                                    *this
                                )
                                    ->start();
                            }
                        );
                    }
                    accept();
                }
            );
        }

        const detail::Storage<Options>& storage;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::ip::tcp::acceptor acceptor;
        std::unordered_map<std::string, Connection*> connections;
    };

    Server::Server(Server::Options options)
        : storage{std::move(options)}
        , impl(std::make_unique<Impl>(storage))
    {
        impl->accept();
    }

    Server::~Server() noexcept = default;

    void Server::run()
    {
        impl->context.run();
    }

    void Server::stop()
    {
        impl->context.stop();
    }

    void Server::restart()
    {
        impl.reset();
        impl = std::make_unique<Impl>(storage);
        impl->accept();
    }

    void Server::on(std::uint32_t type, MsgHandler handler)
    {
        storage.msg.emplace(type, std::move(handler));
    }

    void Server::on(Event event, EventHandler handler)
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

    void Server::send(
        const std::string& id,
        std::uint32_t type,
        const void* data,
        std::uint64_t size
    )
    {
        impl->send(id, type, static_cast<const std::uint8_t*>(data), size);
    }

    void Server::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        impl->publish(type, static_cast<const std::uint8_t*>(data), size);
    }
}
