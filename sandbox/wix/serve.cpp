#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <unordered_map>

const auto path = std::filesystem::path(__FILE__).parent_path();

class Server final
{
public:
    Server()
        : impl(std::make_unique<Impl>(*this))
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
        impl = std::make_unique<Impl>(*this);
        impl->accept();
    }

    void use(std::string route, std::string content_type, std::function<void(std::ostream&)> body)
    {
        this->routes.emplace(std::move(route), Route{std::move(content_type), std::move(body)});
    }

private:
    struct Impl
    {
        Server& parent;
        boost::asio::io_context context;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;

        explicit Impl(Server& init_parent)
            : parent(init_parent)
            , context(1)
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
                        std::make_shared<Connection>(parent, std::move(socket))->start();
                    }
                    accept();
                }
            );
        }
    };

    class Connection: public std::enable_shared_from_this<Connection>
    {
    public:
        explicit Connection(Server& init_parent, boost::asio::ip::tcp::socket init_socket)
            : parent(init_parent)
            , socket(std::move(init_socket))
        {
        }

        void start()
        {
            read_request();
            check_deadline();
        }

    private:
        Server& parent; // NOLINT
        boost::asio::ip::tcp::socket socket;

        boost::beast::flat_buffer buffer{8192};
        boost::beast::http::request<boost::beast::http::dynamic_body> request;
        boost::beast::http::response<boost::beast::http::dynamic_body> response;

        boost::asio::steady_timer deadline_{socket.get_executor(), std::chrono::seconds(60)};

        void read_request()
        {
            boost::beast::http::async_read(
                socket,
                buffer,
                request,
                [self
                 = shared_from_this()](boost::beast::error_code ec, std::size_t bytes_transferred)
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
            response.version(request.version());
            response.keep_alive(false);

            switch (request.method())
            {
                case boost::beast::http::verb::get:
                    response.result(boost::beast::http::status::ok);
                    response.set(boost::beast::http::field::server, "Beast");
                    create_response();
                    break;

                default:
                    response.result(boost::beast::http::status::bad_request);
                    response.set(boost::beast::http::field::content_type, "text/plain");
                    break;
            }

            write_response();
        }

        void create_response()
        {
            const auto it = parent.routes.find(request.target());
            if (it != parent.routes.end())
            {
                const auto& route = it->second;
                response.set(boost::beast::http::field::content_type, it->second.content_type);
                auto os = boost::beast::ostream(response.body());
                route.body(os);
            }
            else
            {
                response.result(boost::beast::http::status::unknown);
                response.set(boost::beast::http::field::content_type, "text/plain");
                boost::beast::ostream(response.body()) << "invalid\r\n";
            }
        }

        void write_response()
        {
            response.content_length(response.body().size());

            boost::beast::http::async_write(
                socket,
                response,
                [self = shared_from_this()](boost::beast::error_code ec, std::size_t)
                {
                    self->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
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
                        self->socket.close(ec);
                    }
                }
            );
        }
    };

    struct Route
    {
        std::string content_type;
        std::function<void(std::ostream&)> body;
    };

    std::unique_ptr<Impl> impl;
    std::unordered_map<std::string, Route> routes;
};

void serve()
{
    Server server;
    server.use(
        "/",
        "text/html",
        [](std::ostream& os)
        {
            std::fstream file(path / "index.html");
            os << file.rdbuf();
        }
    );
    server.use(
        "/index.js",
        "application/javascript",
        [](std::ostream& os)
        {
            std::fstream file(path / "front_end/bundle/index.js");
            os << file.rdbuf();
        }
    );
    server.use(
        "/wix/sample.proto",
        "application/octet-stream",
        [](std::ostream& os)
        {
            std::fstream file(path / "messages/gen/wix/messages/sample.proto");
            os << file.rdbuf();
        }
    );

    server.run();
}
