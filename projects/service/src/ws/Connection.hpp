#pragma once

#include <boost/asio/io_context.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <vector>

namespace nil::service::ws
{
    struct Connection;

    struct IImpl
    {
        virtual void message(std::uint32_t type, const std::uint8_t* data, std::uint64_t size) = 0;
        virtual void connect(Connection* connection) = 0;
        virtual void disconnect(Connection* connection) = 0;
    };

    class Connection final: public std::enable_shared_from_this<Connection>
    {
    public:
        Connection(
            std::uint64_t buffer,
            boost::beast::websocket::stream<boost::beast::tcp_stream> ws,
            IImpl& impl
        );
        ~Connection();

        void start();
        void write(std::uint32_t type, const std::uint8_t* data, std::uint64_t size);
        std::uint16_t id() const;

    private:
        void read();

        boost::beast::websocket::stream<boost::beast::tcp_stream> ws;
        IImpl& impl;
        std::vector<std::uint8_t> buffer;
        boost::beast::flat_static_buffer_base flat_buffer;
    };
}
