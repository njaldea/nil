#pragma once

#include <nil/service/IHandler.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace nil::service::tcp
{
    class Connection: public std::enable_shared_from_this<Connection>
    {
    public:
        Connection(
            boost::asio::io_context& context,
            std::unordered_map<int, std::unique_ptr<IHandler>>& handlers,
            std::unordered_set<Connection*>* parent = nullptr
        );
        ~Connection();

        void start();
        void write(std::string message);
        boost::asio::ip::tcp::socket& handle();

    private:
        void readHeader(std::uint64_t pos, std::uint64_t size);
        void readBody(std::uint64_t pos, std::uint64_t size);

        boost::asio::ip::tcp::socket socket;
        std::unordered_map<int, std::unique_ptr<IHandler>>& handlers;
        std::unordered_set<Connection*>* parent;
        std::vector<char> buffer;
    };
}
