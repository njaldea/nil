#include "Connection.hpp"

#include "../utils.hpp"

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
        read();
    }

    Connection::~Connection() = default;

    void Connection::read()
    {
        ws.async_read(
            flat_buffer,
            [this](boost::beast::error_code ec, std::size_t count)
            {
                if (ec)
                {
                    impl.disconnect(this);
                    return;
                }

                impl.message(id(), r_buffer.data(), count);
                flat_buffer.consume(count);
                read();
            }
        );
    }

    void Connection::write(const std::uint8_t* data, std::uint64_t size)
    {
        boost::system::error_code ec;
        ws.write(boost::asio::buffer(data, size), ec);
    }

    const std::string& Connection::id() const
    {
        return identifier;
    }
}
