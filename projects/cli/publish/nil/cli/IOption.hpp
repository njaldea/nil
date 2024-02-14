#pragma once

namespace nil::cli
{
    struct IOption
    {
        struct Impl;
        virtual ~IOption() noexcept = default;
        virtual void apply(const Impl& impl) const = 0;
    };
}
