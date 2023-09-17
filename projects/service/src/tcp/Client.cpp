#include <nil/service/tcp/Client.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

namespace nil::service::tcp
{
    struct Client::Impl
    {
        Impl(std::string_view host, std::uint16_t port)
            : context()
            , endpoint(boost::asio::ip::make_address(host.data()), port)
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
            static char data[max_length];
            socket.async_read_some(
                boost::asio::buffer(data, max_length),
                [this](const boost::system::error_code& ec, std::size_t count)
                {
                    (void)count;
                    if (ec)
                    {
                        this->reconnect();
                        return;
                    }

                    this->read();
                }
            );
        }

        void write()
        {
            const auto max_length = 1024;
            this->socket.async_write_some(
                boost::asio::buffer("Hello From Client!", max_length),
                [this]( //
                    const boost::system::error_code& ec,
                    size_t count
                )
                {
                    (void)ec;
                    (void)count;
                    // message sent.. what to do? normally nothing.
                }
            );
        }

        boost::asio::io_context context;
        boost::asio::ip::tcp::endpoint endpoint;
        boost::asio::ip::tcp::socket socket;
        boost::asio::deadline_timer reconnection;
    };

    Client::Client(std::string host, int port)
        : mImpl(std::make_unique<Impl>(std::move(host), port))
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
}
