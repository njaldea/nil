#pragma once

namespace nil::utils::traits
{
    template <typename T>
    class type
    {
    private:
        static constexpr const int identifier = 0;

    public:
        static constexpr const void* const value = static_cast<const void*>(&identifier);
    };
}
