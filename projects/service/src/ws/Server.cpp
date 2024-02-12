#include <nil/service/ws/Server.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::ws
{
    struct Server::Impl final: IImpl
    {
        explicit Impl(const detail::Storage<Options>& init_usage)
            : storage(init_usage)
            , strand(boost::asio::make_strand(context))
            , endpoint(boost::asio::ip::make_address("0.0.0.0"), storage.options.port)
            , acceptor(strand, endpoint, true)
        {
        }

        ~Impl() override = default;

        Impl(Impl&&) = delete;
        Impl(const Impl&) = delete;
        Impl& operator=(Impl&&) = delete;
        Impl& operator=(const Impl&) = delete;

        void send(const std::string& id, std::vector<std::uint8_t> data)
        {
            boost::asio::dispatch(
                strand,
                [this, id, msg = std::move(data)]()
                {
                    const auto it = connections.find(id);
                    if (it != connections.end())
                    {
                        it->second->write(msg.data(), msg.size());
                    }
                }
            );
        }

        void publish(std::vector<std::uint8_t> data)
        {
            boost::asio::dispatch(
                strand,
                [this, msg = std::move(data)]()
                {
                    for (const auto& item : connections)
                    {
                        item.second->write(msg.data(), msg.size());
                    }
                }
            );
        }

        void disconnect(Connection* connection) override
        {
            boost::asio::dispatch(
                strand,
                [this, id = connection->id()]()
                {
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

        void message(const std::string& id, const std::uint8_t* data, std::uint64_t size) override
        {
            if (storage.msg)
            {
                storage.msg(id, data, size);
            }
        }

        void accept()
        {
            acceptor.async_accept(
                boost::asio::make_strand(context),
                [this](
                    const boost::system::error_code& acceptor_ec,
                    boost::asio::ip::tcp::socket socket
                )
                {
                    if (!acceptor_ec)
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
                                auto connection = std::make_unique<Connection>(
                                    storage.options.buffer,
                                    std::move(*ws),
                                    *this
                                );
                                auto id = connection->id();
                                connections.emplace(std::move(id), std::move(connection));
                                if (storage.connect)
                                {
                                    storage.connect(id);
                                }
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
        std::unordered_map<std::string, std::unique_ptr<Connection>> connections;
    };

    Server::Server(Server::Options options)
        : storage{options}
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

    void Server::on_message(MessageHandler handler)
    {
        storage.msg = std::move(handler);
    }

    void Server::on_connect(ConnectHandler handler)
    {
        storage.connect = std::move(handler);
    }

    void Server::on_disconnect(DisconnectHandler handler)
    {
        storage.disconnect = std::move(handler);
    }

    void Server::send(const std::string& id, std::vector<std::uint8_t> data)
    {
        impl->send(id, std::move(data));
    }

    void Server::publish(std::vector<std::uint8_t> data)
    {
        impl->publish(std::move(data));
    }
}
