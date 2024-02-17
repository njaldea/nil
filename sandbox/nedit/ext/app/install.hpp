#pragma once

#include "App.hpp"

namespace ext
{
    void install(App& app, std::function<void(std::function<void()>)> rerun);
}
