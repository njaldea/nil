#pragma once

#include "Client.hpp"
#include "Server.hpp"

namespace nil::service::ws
{
    struct modes
    {
        using Server = ws::Server;
        using Client = ws::Client;
    };
}
