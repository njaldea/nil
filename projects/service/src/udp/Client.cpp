#include <nil/service/udp/Client.hpp>

#include "../utils.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

namespace nil::service::udp
{
    struct Client::Impl final
    {
        explicit Impl(const detail::Storage<Options>& init_storage)
            : storage(init_storage)
            , strand(boost::asio::make_strand(context))
            , socket(strand, {boost::asio::ip::make_address("0.0.0.0"), 0})
            , pingtimer(strand)
            , timeout(strand)
            , targetID(storage.options.host + ":" + std::to_string(storage.options.port))
        {
            buffer.resize(storage.options.buffer);
        }

        ~Impl() noexcept = default;

        Impl(Impl&&) noexcept = delete;
        Impl& operator=(Impl&&) noexcept = delete;

        Impl(const Impl&) = delete;
        Impl& operator=(const Impl&) = delete;

        void publish(std::vector<std::uint8_t> data)
        {
            boost::asio::dispatch(
                strand,
                [this, msg = std::move(data)]()
                {
                    socket.send_to(
                        std::array<boost::asio::const_buffer, 2>{
                            boost::asio::buffer(utils::to_array(utils::UDP_EXTERNAL_MESSAGE)),
                            boost::asio::buffer(msg)
                        },
                        {boost::asio::ip::make_address(storage.options.host), storage.options.port}
                    );
                }
            );
        }

        void usermsg(const std::uint8_t* data, std::uint64_t size)
        {
            if (storage.msg)
            {
                storage.msg->call(targetID, data, size);
            }
        }

        void pong()
        {
            if (!connected)
            {
                connected = true;
                if (storage.connect)
                {
                    storage.connect->call(targetID);
                }
            }
            timeout.expires_after(storage.options.timeout);
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
                        if (storage.disconnect)
                        {
                            storage.disconnect->call(targetID);
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
                {boost::asio::ip::make_address(storage.options.host), storage.options.port}
            );
            pingtimer.expires_after(storage.options.timeout / 2);
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

        const detail::Storage<Options>& storage;

        boost::asio::io_context context;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::ip::udp::socket socket;
        boost::asio::steady_timer pingtimer;
        boost::asio::steady_timer timeout;

        std::vector<std::uint8_t> buffer;

        bool connected = false;
        std::string targetID;
    };

    Client::Client(Client::Options options)
        : storage{std::move(options), {}, {}, {}}
        , impl(std::make_unique<Impl>(storage))
    {
        impl->prepare();
    }

    Client::~Client() noexcept = default;

    void Client::run()
    {
        impl->context.run();
    }

    void Client::stop()
    {
        impl->context.stop();
    }

    void Client::restart()
    {
        impl.reset();
        impl = std::make_unique<Impl>(storage);
        impl->prepare();
    }

    void Client::on_message_impl(MessageHandler handler)
    {
        storage.msg = std::move(handler);
    }

    void Client::on_connect_impl(ConnectHandler handler)
    {
        storage.connect = std::move(handler);
    }

    void Client::on_disconnect_impl(DisconnectHandler handler)
    {
        storage.disconnect = std::move(handler);
    }

    void Client::send(const std::string& id, std::vector<std::uint8_t> data)
    {
        if (impl->targetID == id)
        {
            impl->publish(std::move(data));
        }
    }

    void Client::publish(std::vector<std::uint8_t> data)
    {
        impl->publish(std::move(data));
    }
}
