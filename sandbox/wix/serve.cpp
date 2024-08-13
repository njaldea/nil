#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

const auto path = std::filesystem::path(__FILE__).parent_path();

class http_connection: public std::enable_shared_from_this<http_connection>
{
public:
    explicit http_connection(boost::asio::ip::tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    void start()
    {
        read_request();
        check_deadline();
    }

private:
    tcp::socket socket_;

    boost::beast::flat_buffer buffer_{8192};
    boost::beast::http::request<boost::beast::http::dynamic_body> request_;
    boost::beast::http::response<boost::beast::http::dynamic_body> response_;

    boost::asio::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(60)};

    void read_request()
    {
        boost::beast::http::async_read(
            socket_,
            buffer_,
            request_,
            [self = shared_from_this()](boost::beast::error_code ec, std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if (!ec)
                {
                    self->process_request();
                }
            }
        );
    }

    void process_request()
    {
        response_.version(request_.version());
        response_.keep_alive(false);

        switch (request_.method())
        {
            case boost::beast::http::verb::get:
                response_.result(boost::beast::http::status::ok);
                response_.set(boost::beast::http::field::server, "Beast");
                create_response();
                break;

            default:
                response_.result(boost::beast::http::status::bad_request);
                response_.set(boost::beast::http::field::content_type, "text/plain");
                break;
        }

        write_response();
    }

    void create_response()
    {
        // TODO: embed the files in the library
        if (request_.target() == "/")
        {
            response_.set(boost::beast::http::field::content_type, "text/html");
            std::fstream file(path / "index.html");
            boost::beast::ostream(response_.body()) << file.rdbuf();
        }
        else if (request_.target() == "/index.js")
        {
            response_.set(boost::beast::http::field::content_type, "application/javascript");
            std::fstream file(path / "index.js");
            boost::beast::ostream(response_.body()) << file.rdbuf();
        }
        else if (request_.target() == "/wix/sample.proto")
        {
            response_.set(boost::beast::http::field::content_type, "application/octet-stream");
            std::fstream file(path / "messages/gen/wix/messages/sample.proto");
            boost::beast::ostream(response_.body()) << file.rdbuf();
        }
        else
        {
            response_.result(boost::beast::http::status::unknown);
            response_.set(boost::beast::http::field::content_type, "text/plain");
            boost::beast::ostream(response_.body()) << "invalid\r\n";
        }
    }

    void write_response()
    {
        response_.content_length(response_.body().size());

        boost::beast::http::async_write(
            socket_,
            response_,
            [self = shared_from_this()](boost::beast::error_code ec, std::size_t)
            {
                self->socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            }
        );
    }

    void check_deadline()
    {
        deadline_.async_wait(
            [self = shared_from_this()](boost::beast::error_code ec)
            {
                if (!ec)
                {
                    self->socket_.close(ec);
                }
            }
        );
    }
};

struct Server final
{
    Server()
        : impl(std::make_unique<Impl>())
    {
        impl->accept();
    }

    void run() const
    {
        impl->context.run();
    }

    void stop() const
    {
        impl->context.stop();
    }

    void restart()
    {
        impl.reset();
        impl = std::make_unique<Impl>();
        impl->accept();
    }

private:
    struct Impl
    {
        boost::asio::io_context context;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;

        Impl()
            : context(1)
            , acceptor(context, {boost::beast::net::ip::make_address("0.0.0.0"), 8080})
            , socket(context)
        {
        }

        void accept()
        {
            acceptor.async_accept(
                socket,
                [&](boost::beast::error_code ec)
                {
                    if (!ec)
                    {
                        std::make_shared<http_connection>(std::move(socket))->start();
                    }
                    accept();
                }
            );
        }
    };

    std::unique_ptr<Impl> impl;
};

void serve()
{
    Server server;
    server.run();
}
