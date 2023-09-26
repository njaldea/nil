#include <nil/service/udp/Server.hpp>

#include "../Utils.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>

namespace nil::service::udp
{
    struct Server::Impl final
    {
        explicit Impl(Server::Options options)
            : options(options)
            , socket(context, {boost::asio::ip::make_address("0.0.0.0"), this->options.port})
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
        boost::asio::ip::udp::socket socket;

        using EndPoints = std::unordered_map<
            boost::asio::ip::udp::endpoint,
            std::unique_ptr<boost::asio::steady_timer>>;
        EndPoints endpoints;

        std::vector<char> buffer;

        void send(
            std::uint8_t internal,
            std::uint16_t id,
            std::uint32_t type,
            const void* data,
            std::uint64_t size
        )
        {
            context.dispatch(
                [this, internal, type, id, msg = std::string(static_cast<const char*>(data), size)](
                )
                {
                    for (const auto& endpoint : endpoints)
                    {
                        if (endpoint.first.port() == id)
                        {
                            socket.send_to(
                                std::array<boost::asio::const_buffer, 3>{
                                    boost::asio::buffer(utils::to_array(internal)),
                                    boost::asio::buffer(utils::to_array(type)),
                                    boost::asio::buffer(msg.data(), msg.size())
                                },
                                endpoint.first
                            );
                            break;
                        }
                    }
                }
            );
        }

        void publish(
            std::uint8_t internal,
            std::uint32_t type,
            const void* data,
            std::uint64_t size
        )
        {
            auto msg = std::string(static_cast<const char*>(data), size);
            context.dispatch(
                [this,
                 i = utils::to_array(internal),
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

        void usermsg(const char* data, std::uint64_t size)
        {
            const auto it = handlers.msg.find(utils::from_array<std::uint32_t>(data));
            if (it != handlers.msg.end() && it->second)
            {
                it->second(data + sizeof(std::uint32_t), size - sizeof(std::uint32_t));
            }
        }

        void ping(const boost::asio::ip::udp::endpoint& endpoint)
        {
            const auto port = endpoint.port();
            auto& timer = endpoints[endpoint];
            if (!timer)
            {
                timer = std::make_unique<boost::asio::steady_timer>(context);
                if (handlers.connect)
                {
                    handlers.connect(port);
                }
            }
            timer->expires_from_now(options.timeout);
            timer->async_wait(
                [this, port](const boost::system::error_code& ec)
                {
                    if (ec == boost::asio::error::operation_aborted)
                    {
                        return;
                    }
                    if (handlers.disconnect)
                    {
                        handlers.disconnect(port);
                    }
                }
            );

            send(UDP_INTERNAL_MESSAGE, port, 0, nullptr, 0);
        }

        void message(
            const boost::asio::ip::udp::endpoint& endpoint,
            const char* data,
            std::uint64_t size
        )
        {
            if (size >= UDP_REQUIRED_SIZE)
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

        void start()
        {
            auto receiver = std::make_unique<boost::asio::ip::udp::endpoint>();
            auto& capture = *receiver;
            socket.async_receive_from(
                boost::asio::buffer(buffer.data(), buffer.size()),
                capture,
                [this, receiver = std::move(receiver)](
                    const boost::system::error_code& ec,
                    std::size_t count //
                )
                {
                    if (!ec)
                    {
                        this->message(*receiver, static_cast<const char*>(buffer.data()), count);
                        this->start();
                    }
                }
            );
        }
    };

    Server::Server(Server::Options options)
        : mImpl(std::make_unique<Impl>(options))
    {
    }

    Server::~Server() noexcept = default;

    void Server::start()
    {
        mImpl->start();
        mImpl->context.run();
    }

    void Server::stop()
    {
        mImpl->context.stop();
    }

    void Server::on(std::uint32_t type, MsgHandler handler)
    {
        mImpl->handlers.msg.emplace(type, std::move(handler));
    }

    void Server::on(Event event, EventHandler handler)
    {
        switch (event)
        {
            case Event::Connect:
                mImpl->handlers.connect = std::move(handler);
                break;
            case Event::Disconnect:
                mImpl->handlers.disconnect = std::move(handler);
                break;
            default:
                throw std::runtime_error("unknown type");
        }
    }

    void Server::send(std::uint16_t id, std::uint32_t type, const void* data, std::uint64_t size)
    {
        mImpl->send(UDP_EXTERNAL_MESSAGE, id, type, data, size);
    }

    void Server::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        mImpl->publish(UDP_EXTERNAL_MESSAGE, type, data, size);
    }
}
