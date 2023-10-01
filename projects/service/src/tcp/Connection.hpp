#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <vector>

namespace nil::service::tcp
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
        Connection(std::uint64_t buffer, boost::asio::io_context& context, IImpl& impl);
        ~Connection();

        void connected();
        void write(std::uint32_t type, const std::uint8_t* data, std::uint64_t size);
        boost::asio::ip::tcp::socket& handle();

    private:
        void readHeader(std::uint64_t pos, std::uint64_t size);
        void readBody(std::uint64_t pos, std::uint64_t size);

        boost::asio::ip::tcp::socket socket;
        IImpl& impl;
        std::vector<std::uint8_t> buffer;
    };
}
