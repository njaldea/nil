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
    using EventHandler = std::function<void()>;
}
