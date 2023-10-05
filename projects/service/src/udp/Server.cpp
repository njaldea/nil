#include <nil/service/udp/Server.hpp>

#include "../Utils.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::udp
{
    struct Server::Impl final
    {
        explicit Impl(const detail::Storage<Options>& storage)
            : storage(storage)
            , strand(boost::asio::make_strand(context))
            , socket(strand, {boost::asio::ip::make_address("0.0.0.0"), storage.options.port})
        {
            buffer.resize(storage.options.buffer);
        }

        std::string id() const
        {
            return socket.remote_endpoint().address().to_string() + ":"
                + std::to_string(socket.remote_endpoint().port());
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
                [this, type, id, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    for (const auto& connection : connections)
                    {
                        namespace basio = boost::asio;
                        socket.send_to(
                            std::array<basio::const_buffer, 3>{
                                basio::buffer(utils::to_array(utils::UDP_EXTERNAL_MESSAGE)),
                                basio::buffer(utils::to_array(type)),
                                basio::buffer(msg)
                            },
                            connection.second->endpoint
                        );
                    }
                }
            );
        }

        void publish(std::uint32_t type, const std::uint8_t* data, std::uint64_t size)
        {
            auto msg = std::vector<std::uint8_t>(data, data + size);
            boost::asio::dispatch(
                strand,
                [this,
                 i = utils::to_array(utils::UDP_EXTERNAL_MESSAGE),
                 t = utils::to_array(type),
                 msg = std::move(msg)]()
                {
                    const auto b = std::array<boost::asio::const_buffer, 3>{
                        boost::asio::buffer(i),
                        boost::asio::buffer(t),
                        boost::asio::buffer(msg)
                    };
                    for (const auto& connection : connections)
                    {
                        socket.send_to(b, connection.second->endpoint);
                    }
                }
            );
        }

        void ping(const boost::asio::ip::udp::endpoint& endpoint, const std::string& id)
        {
            auto& connection = connections[id];
            if (!connection)
            {
                connection = std::make_unique<Connection>(endpoint, strand);
                if (storage.connect)
                {
                    storage.connect(id);
                }
            }
            connection->timer.expires_after(storage.options.timeout);
            connection->timer.async_wait(
                [this, endpoint, id](const boost::system::error_code& ec)
                {
                    if (ec == boost::asio::error::operation_aborted)
                    {
                        return;
                    }
                    if (storage.disconnect)
                    {
                        storage.disconnect(id);
                    }
                    connections.erase(id);
                }
            );

            socket.send_to(
                boost::asio::buffer(utils::to_array(utils::UDP_INTERNAL_MESSAGE)),
                endpoint
            );
        }

        void usermsg(const std::uint8_t* data, std::uint64_t size)
        {
            if (size >= sizeof(std::uint32_t))
            {
                const auto it = storage.msg.find(utils::from_array<std::uint32_t>(data));
                if (it != storage.msg.end() && it->second)
                {
                    it->second(data + sizeof(std::uint32_t), size - sizeof(std::uint32_t));
                }
            }
        }

        void message(
            const boost::asio::ip::udp::endpoint& endpoint,
            const std::uint8_t* data,
            std::uint64_t size
        )
        {
            if (size >= sizeof(std::uint8_t))
            {
                if (utils::from_array<std::uint8_t>(data) > 0u)
                {
                    ping(
                        endpoint,
                        endpoint.address().to_string() + ":" + std::to_string(endpoint.port())
                    );
                }
                else
                {
                    usermsg(data + sizeof(std::uint8_t), size - sizeof(std::uint8_t));
                }
            }
        }

        void receive()
        {
            auto receiver = std::make_unique<boost::asio::ip::udp::endpoint>();
            auto& capture = *receiver;
            socket.async_receive_from(
                boost::asio::buffer(buffer),
                capture,
                [this, receiver = std::move(receiver)](
                    const boost::system::error_code& ec,
                    std::size_t count //
                )
                {
                    if (!ec)
                    {
                        this->message(*receiver, buffer.data(), count);
                        this->receive();
                    }
                }
            );
        }

        const detail::Storage<Options>& storage;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::ip::udp::socket socket;

        struct Connection
        {
            Connection(
                boost::asio::ip::udp::endpoint endpoint,
                boost::asio::strand<boost::asio::io_context::executor_type>& strand
            )
                : endpoint(std::move(endpoint))
                , timer(strand)
            {
            }

            boost::asio::ip::udp::endpoint endpoint;
            boost::asio::steady_timer timer;
        };

        using Connections = std::unordered_map<std::string, std::unique_ptr<Connection>>;
        Connections connections;

        std::vector<std::uint8_t> buffer;
    };

    Server::Server(Server::Options options)
        : storage{options}
        , impl(std::make_unique<Impl>(storage))
    {
        impl->receive();
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
        impl->receive();
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
