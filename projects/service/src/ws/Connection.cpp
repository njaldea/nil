#include "Connection.hpp"

#include "../Utils.hpp"

namespace nil::service::ws
{
    Connection::Connection(
        std::uint64_t buffer,
        boost::beast::websocket::stream<boost::beast::tcp_stream> ws,
        IImpl& impl
    )
        : ws(std::move(ws))
        , impl(impl)
        , flat_buffer(
              [](std::vector<std::uint8_t>& r, std::vector<std::uint8_t>& w, std::size_t size)
              {
                  r.resize(size);
                  w.resize(size);
                  return boost::beast::flat_static_buffer_base(r.data(), r.size());
              }(r_buffer, w_buffer, buffer)
          )
    {
        impl.connect(this);
    }

    Connection::~Connection()
    {
        impl.disconnect(this);
    }

    void Connection::start()
    {
        read();
    }

    void Connection::read()
    {
        ws.async_read(
            flat_buffer,
            [self = shared_from_this()](boost::beast::error_code ec, std::size_t count)
            {
                if (ec)
                {
                    return;
                }

                self->impl.message(
                    utils::from_array<std::uint32_t>(self->r_buffer.data()),
                    self->r_buffer.data() + sizeof(std::uint32_t),
                    count - sizeof(std::uint32_t)
                );
                self->flat_buffer.consume(count);
                self->read();
            }
        );
    }

    void Connection::write(std::uint32_t type, const std::uint8_t* data, std::uint64_t size)
    {
        const auto t = utils::to_array(type);
        std::memcpy(w_buffer.data(), t.begin(), t.size());
        std::memcpy(w_buffer.data() + t.size(), data, size);
        ws.async_write(
            boost::asio::buffer(w_buffer.data(), t.size() + size),
            [](boost::system::error_code ec, std::size_t count)
            {
                (void)ec;
                (void)count;
            }
        );
    }

    std::uint16_t Connection::id() const
    {
        return boost::beast::get_lowest_layer(ws).socket().remote_endpoint().port();
    }
}
