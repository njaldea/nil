#include "Connection.hpp"

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
        readHeader(0u, 8u);
    }

    void Connection::readHeader(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(buffer.data() + pos, size - pos),
            [pos, size, self = shared_from_this()]( //
                const boost::system::error_code& ec,
                std::size_t count
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
                    for (auto i = 0u; i < 8u; ++i)
                    {
                        esize |= std::uint64_t(self->buffer[i]) << (i * 8);
                    }

                    self->readBody(0, esize);
                }
            }
        );
    }

    void Connection::readBody(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(buffer.data() + pos, size - pos),
            [pos, size, self = shared_from_this()]( //
                const boost::system::error_code& ec,
                std::size_t count
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
                    self->readHeader(0u, 8u);
                }
            }
        );
    }

    void Connection::write(const void* data, std::uint64_t size)
    {
        std::uint8_t header[8];
        for (auto i = 0u; i < 8u; ++i)
        {
            header[i] = std::uint8_t(size >> (i * 8));
        }

        socket.write_some(boost::asio::buffer(header, 8));
        socket.write_some(boost::asio::buffer(data, size));
    }

    boost::asio::ip::tcp::socket& Connection::handle()
    {
        return socket;
    }
}
