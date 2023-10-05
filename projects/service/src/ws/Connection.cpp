#include "Connection.hpp"

#include "../Utils.hpp"

namespace nil::service::ws
{
    Connection::Connection(
        std::uint64_t buffer,
        boost::beast::websocket::stream<boost::beast::tcp_stream> ws,
        IImpl& impl
    )
        : identifier(utils::to_string(         //
            boost::beast::get_lowest_layer(ws) //
                .socket()
                .remote_endpoint()
        ))
        , ws(std::move(ws))
        , impl(impl)
        , flat_buffer(
              [](std::vector<std::uint8_t>& r, std::size_t size)
              {
                  r.resize(size);
                  return boost::beast::flat_static_buffer_base(r.data(), r.size());
              }(r_buffer, buffer)
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
        boost::system::error_code ec;
        ws.write(
            std::array<boost::asio::const_buffer, 2>{
                boost::asio::buffer(utils::to_array(type)),
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
