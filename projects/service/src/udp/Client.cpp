#include <nil/service/udp/Client.hpp>

#include "../Utils.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::udp
{
    struct Client::Impl final
    {
        explicit Impl(Client::Options options)
            : options(std::move(options))
            , strand(boost::asio::make_strand(context))
            , socket(strand, {boost::asio::ip::make_address("0.0.0.0"), 0})
            , pingtimer(strand)
            , timeout(strand)
        {
            buffer.resize(this->options.buffer);
        }

        struct Handlers
        {
            std::unordered_map<std::uint32_t, MsgHandler> msg;
            EventHandler connect;
            EventHandler disconnect;
        } handlers;

        Client::Options options;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::ip::udp::socket socket;
        boost::asio::steady_timer pingtimer;
        boost::asio::steady_timer timeout;

        std::vector<std::uint8_t> buffer;

        bool connected = false;

        void publish(std::uint32_t type, const std::uint8_t* data, std::uint64_t size)
        {
            boost::asio::dispatch(
                strand,
                [this, type, msg = std::vector<std::uint8_t>(data, data + size)]()
                {
                    socket.send_to(
                        std::array<boost::asio::const_buffer, 3>{
                            boost::asio::buffer(utils::to_array(utils::UDP_EXTERNAL_MESSAGE)),
                            boost::asio::buffer(utils::to_array(type)),
                            boost::asio::buffer(msg)
                        },
                        {boost::asio::ip::make_address(options.host), options.port}
                    );
                }
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

        void message(const std::uint8_t* data, std::uint64_t size)
        {
            if (size >= sizeof(std::uint8_t))
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
                boost::asio::buffer(buffer),
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
            socket.send_to(
                boost::asio::buffer(utils::to_array(utils::UDP_INTERNAL_MESSAGE)),
                {boost::asio::ip::make_address(options.host), options.port}
            );
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

        void prepare()
        {
            ping();
            receive();
        }
    };

    Client::Client(Client::Options options)
        : impl(std::make_unique<Impl>(std::move(options)))
    {
    }

    Client::~Client() noexcept = default;

    void Client::prepare()
    {
        impl->prepare();
    }

    void Client::run()
    {
        impl->context.run();
    }

    void Client::stop()
    {
        impl->context.stop();
    }

    void Client::on(std::uint32_t type, MsgHandler handler)
    {
        impl->handlers.msg.emplace(type, std::move(handler));
    }

    void Client::on(Event event, EventHandler handler)
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

    void Client::send(std::uint16_t id, std::uint32_t type, const void* data, std::uint64_t size)
    {
        if (id == impl->options.port)
        {
            impl->publish(type, static_cast<const std::uint8_t*>(data), size);
        }
    }

    void Client::publish(std::uint32_t type, const void* data, std::uint64_t size)
    {
        impl->publish(type, static_cast<const std::uint8_t*>(data), size);
    }
}
