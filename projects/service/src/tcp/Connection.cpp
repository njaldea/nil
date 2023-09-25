#include "Connection.hpp"

namespace
{
    constexpr auto HEADER_SIZE = 8u;
    constexpr auto START_INDEX = 0u;
    constexpr auto TO_BITS = 8u;
}

namespace nil::service::tcp
{
    Connection::Connection(std::uint64_t buffer, boost::asio::io_context& context, IImpl& impl)
        : socket(context)
        , impl(impl)
    {
        this->buffer.resize(buffer);
    }

    Connection::~Connection()
    {
        impl.disconnect(this);
    }

    void Connection::connected()
    {
        impl.connect(this);
        readHeader(0u, HEADER_SIZE);
    }

    void Connection::readHeader(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(buffer.data() + pos, size - pos),
            [pos, size, self = shared_from_this()](
                const boost::system::error_code& ec,
                std::size_t count //
            )
            {
                if (ec)
                {
                    return;
                }

                if (pos + count != size)
                {
                    self->readHeader(pos + count, size);
                }
                else
                {
                    std::uint64_t esize = 0;
                    for (auto i = 0u; i < HEADER_SIZE; ++i)
                    {
                        esize |= std::uint64_t(self->buffer[i]) << (i * TO_BITS);
                    }

                    self->readBody(START_INDEX, esize);
                }
            }
        );
    }

    void Connection::readBody(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(buffer.data() + pos, size - pos),
            [pos, size, self = shared_from_this()](
                const boost::system::error_code& ec,
                std::size_t count //
            )
            {
                if (ec)
                {
                    return;
                }

                if (pos + count != size)
                {
                    self->readBody(pos + count, size);
                }
                else
                {
                    self->impl.message(self->buffer.data(), size);
                    self->readHeader(START_INDEX, HEADER_SIZE);
                }
            }
        );
    }

    void Connection::write(const void* data, std::uint64_t size)
    {
        std::array<std::uint8_t, HEADER_SIZE> header = {
            std::uint8_t(size >> (0 * TO_BITS)),
            std::uint8_t(size >> (1 * TO_BITS)),
            std::uint8_t(size >> (2 * TO_BITS)),
            std::uint8_t(size >> (3 * TO_BITS)),
            std::uint8_t(size >> (4 * TO_BITS)),
            std::uint8_t(size >> (5 * TO_BITS)),
            std::uint8_t(size >> (6 * TO_BITS)),
            std::uint8_t(size >> (7 * TO_BITS)) //
        };

        boost::system::error_code ec;
        socket.write_some(boost::asio::buffer(header, HEADER_SIZE), ec);
        if (!ec)
        {
            socket.write_some(boost::asio::buffer(data, size), ec);
        }
    }

    boost::asio::ip::tcp::socket& Connection::handle()
    {
        return socket;
    }
}
