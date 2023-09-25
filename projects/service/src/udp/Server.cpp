#include <nil/service/udp/Server.hpp>

#include <nil_service_message.pb.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>

namespace nil::service::udp
{
    struct Server::Impl final
    {
        Impl(Server::Options options)
            : options(std::move(options))
            , context()
            , socket(context, {boost::asio::ip::make_address("0.0.0.0"), this->options.port})
        {
            buffer.resize(options.buffer);
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

        std::unordered_map<
            boost::asio::ip::udp::endpoint,
            std::unique_ptr<boost::asio::steady_timer>>
            endpoints;

        std::vector<char> buffer;

        void send(
            bool internal,
            std::uint16_t id,
            std::uint32_t type,
            const void* data,
            std::uint64_t size //
        )
        {
            Message msg;
            msg.set_internal(internal);
            msg.set_type(type);
            msg.set_data(data, size);
            context.dispatch(
                [this, id, msg = msg.SerializeAsString()]()
                {
                    for (const auto& endpoint : endpoints)
                    {
                        if (endpoint.first.port() == id)
                        {
                            socket.send_to(
                                boost::asio::buffer(msg.data(), msg.size()),
                                endpoint.first
                            );
                            break;
                        }
                    }
                }
            );
        }

        void publish(
            bool internal,
            std::uint32_t type,
            const void* data,
            std::uint64_t size //
        )
        {
            Message msg;
            msg.set_internal(internal);
            msg.set_type(type);
            msg.set_data(data, size);
            context.dispatch(
                [this, msg = msg.SerializeAsString()]()
                {
                    for (const auto& endpoint : endpoints)
                    {
                        socket.send_to(boost::asio::buffer(msg.data(), msg.size()), endpoint.first);
                    }
                }
            );
        }

        void usermsg(std::uint32_t type, const void* data, std::uint64_t size)
        {
            const auto it = handlers.msg.find(type);
            if (it != handlers.msg.end())
            {
                if (it->second)
                {
                    it->second(data, size);
                }
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

            publish(true, 0, nullptr, 0);
        }

        void message(
            const boost::asio::ip::udp::endpoint& endpoint,
            const void* data,
            std::uint64_t size
        )
        {
            Message msg;
            msg.ParseFromArray(data, int(size));
            if (msg.internal())
            {
                ping(endpoint);
            }
            else
            {
                const auto& inner = msg.data();
                usermsg(msg.type(), inner.data(), inner.size());
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
                        this->message(*receiver, buffer.data(), count);
                        this->start();
                    }
                }
            );
        }
    };

    Server::Server(Server::Options options)
        : mImpl(std::make_unique<Impl>(std::move(options)))
    {
    }

    Server::~Server() noexcept = default;

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

    void Server::start()
    {
        mImpl->start();
        mImpl->context.run();
    }

    void Server::stop()
    {
        mImpl->context.stop();
    }

    void Server::send(
        std::uint16_t id,
        std::uint32_t type,
        const void* data,
        std::uint64_t size //
    )
    {
        mImpl->send(false, id, type, data, size);
    }

    void Server::publish(
        std::uint32_t type,
        const void* data,
        std::uint64_t size //
    )
    {
        mImpl->publish(false, type, data, size);
    }
}
