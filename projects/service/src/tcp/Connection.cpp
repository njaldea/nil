#include "Connection.hpp"

#include "../Utils.hpp"

namespace nil::service::tcp
{
    Connection::Connection(std::uint64_t buffer, boost::asio::ip::tcp::socket socket, IImpl& impl)
        : socket(std::move(socket))
        , impl(impl)
    {
        this->r_buffer.resize(buffer);
        this->w_buffer.resize(buffer);
        impl.connect(this);
    }

    Connection::~Connection()
    {
        impl.disconnect(this);
    }

    void Connection::start()
    {
        readHeader(0u, utils::TCP_HEADER_SIZE);
    }

    void Connection::readHeader(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(r_buffer.data() + pos, size - pos),
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
                    const auto msgsize = utils::from_array<std::uint64_t>(self->r_buffer.data());
                    self->readBody(utils::START_INDEX, msgsize);
                }
            }
        );
    }

    void Connection::readBody(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(r_buffer.data() + pos, size - pos),
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
                    self->impl.message(
                        utils::from_array<std::uint32_t>(self->r_buffer.data()),
                        self->r_buffer.data() + sizeof(std::uint32_t),
                        size - sizeof(std::uint32_t)
                    );
                    self->readHeader(utils::START_INDEX, utils::TCP_HEADER_SIZE);
                }
            }
        );
    }

    void Connection::write(std::uint32_t type, const std::uint8_t* data, std::uint64_t size)
    {
        const auto s = utils::to_array(size + sizeof(type));
        const auto t = utils::to_array(type);
        std::memcpy(w_buffer.data(), s.begin(), s.size());
        std::memcpy(w_buffer.data() + s.size(), t.begin(), t.size());
        std::memcpy(w_buffer.data() + s.size() + t.size(), data, size);
        socket.async_write_some(
            boost::asio::buffer(w_buffer.data(), s.size() + t.size() + size),
            [](boost::system::error_code ec, std::size_t count)
            {
                (void)ec;
                (void)count;
            }
        );
    }

    std::string Connection::id() const
    {
        return socket.remote_endpoint().address().to_string() + ":"
            + std::to_string(socket.remote_endpoint().port());
    }
}
