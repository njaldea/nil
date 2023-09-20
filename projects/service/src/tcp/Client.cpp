#include <nil/service/tcp/Client.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <iostream>

namespace nil::service::tcp
{
    struct Client::Impl
    {
        Impl(Client::Options options)
            : context()
            , endpoint(boost::asio::ip::make_address(options.host.data()), options.port)
            , socket(context)
            , reconnection(context)
        {
        }

        void connect()
        {
            reconnection.cancel();
            socket.async_connect(
                endpoint,
                [this](const boost::system::error_code& ec)
                {
                    if (ec)
                    {
                        this->reconnect();
                        return;
                    }

                    this->read();
                }
            );
        }

        void reconnect()
        {
            reconnection.expires_from_now(boost::posix_time::milliseconds(25));
            reconnection.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        this->connect();
                        return;
                    }

                    this->read();
                }
            );
        }

        void read()
        {
            const auto max_length = 1024;
            socket.async_read_some(
                boost::asio::buffer(buffer.data(), max_length),
                [this](const boost::system::error_code& ec, std::size_t count)
                {
                    if (ec)
                    {
                        this->reconnect();
                        return;
                    }

                    std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
                    std::cout << std::string(buffer.data(), count) << std::endl;

                    this->read();
                }
            );
        }

        void write(std::string message)
        {
            const auto max_length = 1024;
            const std::uint64_t s = message.size();
            std::uint8_t size[8];
            size[0] = std::uint8_t(s >> (0 * 8));
            size[1] = std::uint8_t(s >> (1 * 8));
            size[2] = std::uint8_t(s >> (2 * 8));
            size[3] = std::uint8_t(s >> (3 * 8));
            size[4] = std::uint8_t(s >> (4 * 8));
            size[5] = std::uint8_t(s >> (5 * 8));
            size[6] = std::uint8_t(s >> (6 * 8));
            size[7] = std::uint8_t(s >> (7 * 8));

            this->socket.async_write_some(
                boost::asio::buffer(size, 8),
                [this]( //
                    const boost::system::error_code& ec,
                    size_t count
                )
                {
                    if (ec)
                    {
                        return;
                    }
                    (void)count;
                }
            );

            this->socket.async_write_some(
                boost::asio::buffer(message, max_length),
                [this]( //
                    const boost::system::error_code& ec,
                    size_t count
                )
                {
                    if (ec)
                    {
                        return;
                    }
                    (void)count;
                    // message sent.. what to do? normally nothing.
                }
            );
        }

        boost::asio::io_context context;

        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::ip::tcp::socket socket;
        boost::asio::deadline_timer reconnection;

        std::vector<char> buffer;
    };

    Client::Client(Client::Options options)
        : mImpl(std::make_unique<Impl>(std::move(options)))
    {
    }

    Client::~Client() noexcept = default;

    void Client::on(int type, std::unique_ptr<IHandler> handler)
    {
        (void)type;
        (void)handler;
    }

    void Client::start()
    {
        mImpl->connect();
        mImpl->context.run();
    }

    void Client::stop()
    {
        mImpl->context.stop();
    }

    void Client::publish(int type, std::string msg)
    {
        (void)type;
        mImpl->context.dispatch([&, msg = std::move(msg)]() { mImpl->write(msg); });
    }
}
