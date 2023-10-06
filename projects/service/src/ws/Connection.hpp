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
        virtual void message( //
            const std::string& id,
            const std::uint8_t* data,
            std::uint64_t size
        ) = 0;
        virtual void disconnect(Connection* connection) = 0;
    };

    class Connection final
    {
    public:
        Connection(
            std::uint64_t buffer,
            boost::beast::websocket::stream<boost::beast::tcp_stream> ws,
            IImpl& impl
        );
        ~Connection();

        void write(const std::uint8_t* data, std::uint64_t size);
        const std::string& id() const;

    private:
        void read();

        std::string identifier;
        boost::beast::websocket::stream<boost::beast::tcp_stream> ws;
        IImpl& impl;
        std::vector<std::uint8_t> r_buffer;
        boost::beast::flat_static_buffer_base flat_buffer;
    };
}
