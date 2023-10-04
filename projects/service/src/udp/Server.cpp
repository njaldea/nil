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
        explicit Impl(Server::Options options)
            : options(options)
            , strand(boost::asio::make_strand(context))
            , socket(strand, {boost::asio::ip::make_address("0.0.0.0"), this->options.port})
        {
            buffer.resize(this->options.buffer);
        }

        Server::Options options;

        struct Handlers
        {
            std::unordered_map<std::uint32_t, MsgHandler> msg;
            EventHandler connect;
            EventHandler disconnect;
        } handlers;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::ip::udp::socket socket;

        using EndPoints = std::unordered_map<
            boost::asio::ip::udp::endpoint,
            std::unique_ptr<boost::asio::steady_timer>>;
        EndPoints endpoints;

        std::vector<std::uint8_t> buffer;

        void send(
            std::uint16_t id,
            std::uint32_t type,
            const std::uint8_t* data,
            std::uint64_t size
        )
        {
            boost::asio::dispatch(
                strand,
                [this, type, id, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    for (const auto& endpoint : endpoints)
                    {
                        if (endpoint.first.port() == id)
                        {
                            namespace basio = boost::asio;
                            socket.send_to(
                                std::array<basio::const_buffer, 3>{
                                    basio::buffer(utils::to_array(utils::UDP_EXTERNAL_MESSAGE)),
                                    basio::buffer(utils::to_array(type)),
                                    basio::buffer(msg)
                                },
                                endpoint.first
                            );
                            break;
                        }
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
                    for (const auto& endpoint : endpoints)
                    {
                        socket.send_to(b, endpoint.first);
                    }
                }
            );
        }

        void ping(const boost::asio::ip::udp::endpoint& endpoint)
        {
            auto& timer = endpoints[endpoint];
            if (!timer)
            {
                timer = std::make_unique<boost::asio::steady_timer>(strand);
                if (handlers.connect)
                {
                    handlers.connect(endpoint.port());
                }
            }
            timer->expires_from_now(options.timeout);
            timer->async_wait(
                [this, endpoint](const boost::system::error_code& ec)
                {
                    if (ec == boost::asio::error::operation_aborted)
                    {
                        return;
                    }
                    if (handlers.disconnect)
                    {
                        handlers.disconnect(endpoint.port());
                    }
                    endpoints.erase(endpoint);
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
                const auto it = handlers.msg.find(utils::from_array<std::uint32_t>(data));
                if (it != handlers.msg.end() && it->second)
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
                    ping(endpoint);
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
    };

    Server::Server(Server::Options options)
        : impl(std::make_unique<Impl>(options))
    {
    }

    Server::~Server() noexcept = default;

    void Server::prepare()
    {
        impl->receive();
    }

    void Server::run()
    {
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
