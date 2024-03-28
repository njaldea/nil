#include <nil/service/udp/Server.hpp>

#include "../utils.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::udp
{
    struct Server::Impl final
    {
        explicit Impl(const detail::Storage<Options>& init_storage)
            : storage(init_storage)
            , strand(boost::asio::make_strand(context))
            , socket(strand, {boost::asio::ip::make_address("0.0.0.0"), storage.options.port})
        {
            buffer.resize(storage.options.buffer);
        }

        ~Impl() noexcept = default;

        Impl(Impl&&) noexcept = delete;
        Impl& operator=(Impl&&) noexcept = delete;

        Impl(const Impl&) = delete;
        Impl& operator=(const Impl&) = delete;

        std::string id() const
        {
            return socket.remote_endpoint().address().to_string() + ":"
                + std::to_string(socket.remote_endpoint().port());
        }

        void send(const std::string& id, std::vector<std::uint8_t> data)
        {
            boost::asio::dispatch(
                strand,
                [this, id, i = utils::to_array(utils::UDP_EXTERNAL_MESSAGE), msg = std::move(data)](
                )
                {
                    const auto b = std::array<boost::asio::const_buffer, 2>{
                        boost::asio::buffer(i),
                        boost::asio::buffer(msg)
                    };
                    for (const auto& [cid, connection] : connections)
                    {
                        if (cid == id)
                        {
                            socket.send_to(b, connection->endpoint);
                        }
                    }
                }
            );
        }

        void publish(std::vector<std::uint8_t> data)
        {
            boost::asio::dispatch(
                strand,
                [this, i = utils::to_array(utils::UDP_EXTERNAL_MESSAGE), msg = std::move(data)]()
                {
                    const auto b = std::array<boost::asio::const_buffer, 3>{
                        boost::asio::buffer(i),
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
                    storage.connect->call(id);
                }
            }
            connection->timer.expires_after(storage.options.timeout);
            connection->timer.async_wait(
                [this, id](const boost::system::error_code& ec)
                {
                    if (ec == boost::asio::error::operation_aborted)
                    {
                        return;
                    }
                    if (storage.disconnect)
                    {
                        storage.disconnect->call(id);
                    }
                    connections.erase(id);
                }
            );

            socket.send_to(
                boost::asio::buffer(utils::to_array(utils::UDP_INTERNAL_MESSAGE)),
                endpoint
            );
        }

        void usermsg(const std::string& id, const std::uint8_t* data, std::uint64_t size)
        {
            if (storage.msg)
            {
                storage.msg->call(id, data, size);
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
                    ping(endpoint, utils::to_string(endpoint));
                }
                else
                {
                    usermsg(
                        utils::to_string(endpoint),
                        data + sizeof(std::uint8_t),
                        size - sizeof(std::uint8_t)
                    );
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
                        message(*receiver, buffer.data(), count);
                        receive();
                    }
                }
            );
        }

        const detail::Storage<Options>& storage;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::ip::udp::socket socket;

        struct Connection final
        {
            Connection(
                boost::asio::ip::udp::endpoint init_endpoint,
                boost::asio::strand<boost::asio::io_context::executor_type>& strand
            )
                : endpoint(std::move(init_endpoint))
                , timer(strand)
            {
            }

            ~Connection() noexcept = default;

            Connection(const Connection&) = delete;
            Connection(Connection&&) noexcept = delete;
            Connection& operator=(const Connection&) = delete;
            Connection& operator=(Connection&&) noexcept = delete;

            boost::asio::ip::udp::endpoint endpoint;
            boost::asio::steady_timer timer;
        };

        using Connections = std::unordered_map<std::string, std::unique_ptr<Connection>>;
        Connections connections;

        std::vector<std::uint8_t> buffer;
    };

    Server::Server(Server::Options options)
        : storage{options, {}, {}, {}}
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

    void Server::on_message_impl(MessageHandler handler)
    {
        storage.msg = std::move(handler);
    }

    void Server::on_connect_impl(ConnectHandler handler)
    {
        storage.connect = std::move(handler);
    }

    void Server::on_disconnect_impl(DisconnectHandler handler)
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
