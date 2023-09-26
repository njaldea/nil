#include <nil/service/udp/Client.hpp>

#include "../Utils.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>

namespace nil::service::udp
{
    struct Client::Impl final
    {
        explicit Impl(Client::Options options)
            : options(std::move(options))
            , socket(context, {boost::asio::ip::make_address("0.0.0.0"), 0})
            , pingtimer(context)
            , timeout(context)
        {
            buffer.resize(options.buffer);
        }

        struct Handlers
        {
            std::unordered_map<std::uint32_t, MsgHandler> msg;
            EventHandler connect;
            EventHandler disconnect;
        } handlers;

        Client::Options options;

        boost::asio::io_context context;
        boost::asio::ip::udp::socket socket;
        boost::asio::steady_timer pingtimer;
        boost::asio::steady_timer timeout;

        std::vector<char> buffer;

        bool connected = false;

        void publish(
            std::uint8_t internal,
            std::uint32_t type,
            const void* data,
            std::uint64_t size
        )
        {
            context.dispatch(
                [this, internal, type, msg = std::string(static_cast<const char*>(data), size)]()
                {
                    socket.send_to(
                        std::array<boost::asio::const_buffer, 3>{
                            boost::asio::buffer(utils::to_array(internal)),
                            boost::asio::buffer(utils::to_array(type)),
                            boost::asio::buffer(msg.data(), msg.size())
                        },
                        {boost::asio::ip::make_address(options.host), options.port}
                    );
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

        void pong()
        {
            if (!connected)
            {
                connected = true;
                if (handlers.connect)
                {
                    handlers.connect(options.port);
                }
            }
            timeout.expires_from_now(options.timeout);
            timeout.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec == boost::asio::error::operation_aborted)
                    {
                        return;
                    }
                    if (connected)
                    {
                        connected = false;
                        if (handlers.disconnect)
                        {
                            handlers.disconnect(options.port);
                        }
                    }
                }
            );
        }

        void message(const char* data, std::uint64_t size)
        {
            if (size >= UDP_REQUIRED_SIZE)
            {
                if (utils::from_array<std::uint8_t>(data) > 0u)
                {
                    pong();
                }
                else
                {
                    usermsg(data + sizeof(std::uint8_t), size - sizeof(std::uint8_t));
                }
            }
        }

        void receive()
        {
            socket.async_receive(
                boost::asio::buffer(buffer.data(), buffer.size()),
                [this](const boost::system::error_code& ec, std::size_t count)
                {
                    if (ec)
                    {
                        return;
                    }

                    message(buffer.data(), count);
                    receive();
                }
            );
        }

        void ping()
        {
            publish(UDP_INTERNAL_MESSAGE, 0, nullptr, 0);
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

    void Client::start()
    {
        mImpl->start();
        mImpl->context.run();
    }

    void Client::stop()
    {
        mImpl->context.stop();
    }

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

    void Client::send(std::uint16_t id, std::uint32_t type, const void* data, std::uint64_t size)
    {
        if (id == mImpl->options.port)
        {
            mImpl->publish(UDP_EXTERNAL_MESSAGE, type, data, size);
        }
    }

    void Client::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        mImpl->publish(UDP_EXTERNAL_MESSAGE, type, data, size);
    }
}
