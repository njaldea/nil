#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <vector>

namespace nil::service::tcp
{
    class Connection;

    struct IImpl
    {
        IImpl() = default;
        virtual ~IImpl() noexcept = default;

        IImpl(IImpl&&) noexcept = delete;
        IImpl& operator=(IImpl&&) noexcept = delete;

        IImpl(const IImpl&) = delete;
        IImpl& operator=(const IImpl&) = delete;

        virtual void message(
            const std::string& id,
            const std::uint8_t* data,
            std::uint64_t size //
        ) = 0;
        virtual void disconnect(Connection* connection) = 0;
    };

    class Connection final
    {
    public:
        Connection(std::uint64_t buffer, boost::asio::ip::tcp::socket socket, IImpl& impl);
        ~Connection() noexcept;

        Connection(Connection&&) noexcept = delete;
        Connection(const Connection&) = delete;
        Connection& operator=(Connection&&) noexcept = delete;
        Connection& operator=(const Connection&) = delete;

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
