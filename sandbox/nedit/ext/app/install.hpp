#pragma once

#include "App.hpp"

namespace ext
{
    void install(App& app, const std::function<void(std::function<void()>)>& post);
}
