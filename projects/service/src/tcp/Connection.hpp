#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <vector>

namespace nil::service::tcp
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
        Connection(std::uint64_t buffer, boost::asio::ip::tcp::socket socket, IImpl& impl);
        ~Connection();

        void write(const std::uint8_t* data, std::uint64_t size);
        const std::string& id() const;

    private:
        void readHeader(std::uint64_t pos, std::uint64_t size);
        void readBody(std::uint64_t pos, std::uint64_t size);

        std::string identifier;
        boost::asio::ip::tcp::socket socket;
        IImpl& impl;
        std::vector<std::uint8_t> r_buffer;
    };
}
