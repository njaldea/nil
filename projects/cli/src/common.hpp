#pragma once

#include <cli/Builder.hpp>

#include <boost/program_options/options_description.hpp>

namespace nil::cli
{
    struct IOption::Impl
    {
        boost::program_options::options_description_easy_init& ex;
    };
}
