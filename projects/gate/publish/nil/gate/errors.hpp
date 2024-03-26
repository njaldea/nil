#pragma once

namespace nil::gate::errors
{
    template <bool T>
    struct Check
    {
    };

    struct Error
    {
        Error(Check<false> tag) = delete;

        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        Error(Check<true> tag)
        {
            (void)tag;
        }
    };
}
