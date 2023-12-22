#include <nil/service/tcp/Server.hpp>

#include "Connection.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::tcp
{
    struct Server::Impl final: IImpl
    {
        explicit Impl(const detail::Storage<Options>& init_usage)
            : storage(init_usage)
            , strand(boost::asio::make_strand(context))
            , acceptor(
                  strand,
                  {boost::asio::ip::make_address("0.0.0.0"), storage.options.port},
                  true
              )
        {
        }

        ~Impl() = default;

        Impl(const Impl&) = delete;
        Impl(Impl&&) = delete;
        Impl& operator=(const Impl&) = delete;
        Impl& operator=(Impl&&) = delete;

        void send(const std::string& id, const std::uint8_t* data, std::uint64_t size)
        {
            boost::asio::dispatch(
                strand,
                [this, id, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    const auto it = connections.find(id);
                    if (it != connections.end())
                    {
                        it->second->write(msg.data(), msg.size());
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
                [this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket)
                {
                    if (!ec)
                    {
                        auto connection = std::make_unique<Connection>(
                            storage.options.buffer,
                            std::move(socket),
                            *this
                        );
                        const auto& id = connection->id();
                        if (storage.connect)
                        {
                            storage.connect(id);
                        }
                        connections.emplace(std::move(id), std::move(connection));
                    }
                    accept();
                }
            );
        }

        const detail::Storage<Options>& storage;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
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

    void Server::send(const std::string& id, const void* data, std::uint64_t size)
    {
        impl->send(id, static_cast<const std::uint8_t*>(data), size);
    }

    void Server::publish(const void* data, std::uint64_t size)
    {
        impl->publish(static_cast<const std::uint8_t*>(data), size);
    }
}
