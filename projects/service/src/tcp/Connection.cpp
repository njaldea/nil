#include "Connection.hpp"

#include "../utils.hpp"

namespace nil::service::tcp
{
    Connection::Connection(
        std::uint64_t buffer,
        boost::asio::ip::tcp::socket init_socket,
        IImpl& init_impl
    )
        : identifier(utils::to_string(init_socket.remote_endpoint()))
        , socket(std::move(init_socket))
        , impl(init_impl)
    {
        this->r_buffer.resize(buffer);
        readHeader(0u, utils::TCP_HEADER_SIZE);
    }

    Connection::~Connection() noexcept = default;

    void Connection::readHeader(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(r_buffer.data() + pos, size - pos),
            [pos, size, this](
                const boost::system::error_code& ec,
                std::size_t count //
            )
            {
                if (ec)
                {
                    impl.disconnect(this);
                    return;
                }

                if (pos + count != size)
                {
                    readHeader(pos + count, size);
                }
                else
                {
                    readBody(utils::START_INDEX, utils::from_array<std::uint64_t>(r_buffer.data()));
                }
            }
        );
    }

    void Connection::readBody(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(r_buffer.data() + pos, size - pos),
            [pos, size, this](const boost::system::error_code& ec, std::size_t count)
            {
                if (ec)
                {
                    impl.disconnect(this);
                    return;
                }

                if (pos + count != size)
                {
                    readBody(pos + count, size);
                }
                else
                {
                    impl.message(id(), r_buffer.data(), size);
                    readHeader(utils::START_INDEX, utils::TCP_HEADER_SIZE);
                }
            }
        );
    }

    void Connection::write(const std::uint8_t* data, std::uint64_t size)
    {
        boost::system::error_code ec;
        socket.write_some(
            std::array<boost::asio::const_buffer, 2>{
                boost::asio::buffer(utils::to_array(size)),
                boost::asio::buffer(data, size)
            },
            ec
        );
    }

    const std::string& Connection::id() const
    {
        return identifier;
    }
}
