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

        void publish(bool user, std::uint32_t type, const void* data, std::uint64_t size)
        {
            Message msg;
            msg.set_user(user);
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

        void message(
            const boost::asio::ip::udp::endpoint& endpoint,
            const void* data,
            std::uint64_t size
        )
        {
            Message msg;
            msg.ParseFromArray(data, int(size));
            if (msg.user())
            {
                const auto it = handlers.msg.find(msg.type());
                if (it != handlers.msg.end())
                {
                    if (it->second)
                    {
                        const auto& inner = msg.data();
                        it->second(inner.data(), inner.size());
                    }
                }
            }
            else
            {
                auto& timer = endpoints[endpoint];
                if (!timer)
                {
                    timer = std::make_unique<boost::asio::steady_timer>(context);
                    if (handlers.connect)
                    {
                        handlers.connect();
                    }
                }
                timer->cancel();
                timer->expires_from_now(options.timeout);
                timer->async_wait(
                    [this](const boost::system::error_code& ec)
                    {
                        if (ec != boost::asio::error::operation_aborted && handlers.disconnect)
                        {
                            handlers.disconnect();
                        }
                    }
                );

                publish(false, 0, nullptr, 0);
            }
        }

        void start()
        {
            static char buffer[1024];
            auto receiver = std::make_shared<boost::asio::ip::udp::endpoint>();
            socket.async_receive_from(
                boost::asio::buffer(buffer, 1024),
                *receiver,
                [this, receiver](const boost::system::error_code& ec, std::size_t count)
                {
                    if (ec)
                    {
                        return;
                    }
                    this->message(*receiver, buffer, count);
                    this->start();
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

    void Server::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        mImpl->publish(true, type, data, size);
    }
}
