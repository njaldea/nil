#pragma once

#include "Client.hpp"
#include "Server.hpp"

namespace nil::service::udp
{
    struct modes
    {
        using Server = udp::Server;
        using Client = udp::Client;
    };
}
