#pragma once

#include <functional>

namespace nil::service
{
    enum class Event
    {
        Connect,
        Disconnect
    };

    // TODO: add wrapper object in MsgHandler to allow response
    using MsgHandler = std::function<void(const void*, std::uint64_t)>;

    /**
     * @brief Handler types for Connect/Disconnect.
     *  Argument is an identifier for the connection.
     *  Currently, it uses the port of the connecting agent.
     */
    using EventHandler = std::function<void(const std::string&)>;

    namespace detail
    {
        template <typename Options>
        struct Storage
        {
            const Options options;
            std::unordered_map<std::uint32_t, MsgHandler> msg = {};
            EventHandler connect = {};
            EventHandler disconnect = {};
        };
    }
}
