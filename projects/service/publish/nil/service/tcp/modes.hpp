#pragma once

#include "Client.hpp"
#include "Server.hpp"

namespace nil::service::tcp
{
    struct modes
    {
        using Server = tcp::Server;
        using Client = tcp::Client;
    };
}
