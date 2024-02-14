#pragma once

#include <nil/cli/Builder.hpp>
#include <nil/cli/IOption.hpp>

namespace boost::program_options
{
    class options_description_easy_init;
}

namespace nil::cli
{
    struct IOption::Impl final
    {
        boost::program_options::options_description_easy_init* ex;
    };
}
