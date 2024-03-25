#pragma once

#include "../traits/edgify.hpp"

#include <memory>
#include <optional>

namespace nil::gate::traits
{
    template <typename T>
    struct edgify<std::unique_ptr<T>>
    {
        using type = std::unique_ptr<const T>;
    };

    template <typename T>
    struct edgify<std::shared_ptr<T>>
    {
        using type = std::shared_ptr<const T>;
    };

    template <typename T>
    struct edgify<std::optional<T>>
    {
        using type = std::optional<const T>;
    };

    template <typename T>
    struct edgify<std::reference_wrapper<T>>
    {
        using type = std::reference_wrapper<const T>;
    };
}
