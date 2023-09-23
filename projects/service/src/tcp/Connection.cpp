#include "Connection.hpp"

#include "msg.pb.h"

namespace nil::service::tcp
{
    Connection::Connection(
        std::uint64_t buffer,
        boost::asio::io_context& context,
        std::unordered_map<std::uint32_t, std::unique_ptr<IHandler>>& handlers,
        std::unordered_set<Connection*>* parent
    )
        : socket(context)
        , handlers(handlers)
        , parent(parent)
    {
        this->buffer.resize(buffer);
    }

    Connection::~Connection()
    {
        if (parent && parent->contains(this))
        {
            parent->erase(this);
        }
    }

    void Connection::start()
    {
        if (parent)
        {
            parent->emplace(this);
        }
        readHeader(0u, 8u);
    }

    void Connection::readHeader(std::uint64_t pos, std::uint64_t size)
    {
        socket.async_read_some(
            boost::asio::buffer(buffer.data() + pos, size - pos),
            [pos, size, self = shared_from_this()]( //
                const boost::system::error_code& ec,
                size_t count
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
                size_t count
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
                    Message message;
                    message.ParseFromArray(self->buffer.data(), int(size));
                    const auto it = self->handlers.find(message.type());
                    if (it != self->handlers.end())
                    {
                        const auto& inner = message.data();
                        it->second->exec(inner.data(), inner.size());
                    }
                    self->readHeader(0u, 8u);
                }
            }
        );
    }

    void Connection::write(std::uint32_t type, std::string message)
    {
        Message proto;
        proto.set_type(type);
        proto.set_data(std::move(message));

        const auto msg = proto.SerializeAsString();
        const auto msgsize = msg.size();

        std::uint8_t size[8];
        for (auto i = 0u; i < 8u; ++i)
        {
            size[i] = std::uint8_t(msgsize >> (i * 8));
        }

        socket.write_some(boost::asio::buffer(size, 8));
        socket.write_some(boost::asio::buffer(msg.data(), msgsize));
    }

    boost::asio::ip::tcp::socket& Connection::handle()
    {
        return socket;
    }
}
