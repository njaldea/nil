#include <nil/service/udp/Client.hpp>

#include <nil_service_message.pb.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>

namespace nil::service::udp
{
    struct Client::Impl final
    {
        Impl(Client::Options options)
            : options(std::move(options))
            , context()
            , socket(context, {boost::asio::ip::make_address("0.0.0.0"), 0})
            , pingtimer(context)
            , timeout(context)
        {
        }

        Client::Options options;

        struct Handlers
        {
            std::unordered_map<std::uint32_t, MsgHandler> msg;
            EventHandler connect;
            EventHandler disconnect;
        } handlers;

        boost::asio::io_context context;
        boost::asio::ip::udp::endpoint endpoint;
        boost::asio::ip::udp::socket socket;
        boost::asio::steady_timer pingtimer;
        boost::asio::steady_timer timeout;

        bool connected = false;

        void publish(bool user, std::uint32_t type, const void* data, std::uint64_t size)
        {
            Message msg;
            msg.set_user(user);
            msg.set_type(type);
            msg.set_data(data, size);
            context.dispatch([this, msg = msg.SerializeAsString()]() { //
                socket.send_to(
                    boost::asio::buffer(msg.data(), msg.size()),
                    {boost::asio::ip::make_address(this->options.host), this->options.port}
                );
            });
        }

        void message(const void* data, std::uint64_t size)
        {
            Message message;
            message.ParseFromArray(data, int(size));
            if (message.user())
            {
                const auto it = handlers.msg.find(message.type());
                if (it != handlers.msg.end())
                {
                    if (it->second)
                    {
                        const auto& inner = message.data();
                        it->second(inner.data(), inner.size());
                    }
                }
            }
            else
            {
                if (!connected)
                {
                    connected = true;
                    if (handlers.connect)
                    {
                        handlers.connect();
                    }
                }
                timeout.cancel();
                timeout.expires_from_now(options.timeout);
                timeout.async_wait(
                    [this](const boost::system::error_code& ec)
                    {
                        if (ec != boost::asio::error::operation_aborted && connected)
                        {
                            connected = false;
                            if (handlers.disconnect)
                            {
                                handlers.disconnect();
                            }
                        }
                    }
                );
            }
        }

        void receive()
        {
            static char buffer[1024];
            socket.async_receive(
                boost::asio::buffer(buffer, 1024),
                [this](const boost::system::error_code& ec, std::size_t count)
                {
                    if (ec)
                    {
                        return;
                    }
                    this->message(buffer, count);
                    this->receive();
                }
            );
        }

        void ping()
        {
            publish(false, 0, nullptr, 0);
            pingtimer.expires_from_now(options.timeout / 2);
            pingtimer.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        ping();
                    }
                }
            );
        }

        void start()
        {
            ping();
            receive();
        }
    };

    Client::Client(Client::Options options)
        : mImpl(std::make_unique<Impl>(std::move(options)))
    {
    }

    Client::~Client() noexcept = default;

    void Client::on(std::uint32_t type, MsgHandler handler)
    {
        mImpl->handlers.msg.emplace(type, std::move(handler));
    }

    void Client::on(Event event, EventHandler handler)
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

    void Client::start()
    {
        mImpl->start();
        mImpl->context.run();
    }

    void Client::stop()
    {
        mImpl->context.stop();
    }

    void Client::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        mImpl->publish(true, type, data, size);
    }
}
