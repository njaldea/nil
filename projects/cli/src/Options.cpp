#include <nil/cli/Options.hpp>

#include "common.hpp"
#include <nil/cli/Node.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iomanip>
#include <ostream>
#include <tuple>

namespace nil::cli
{
    struct Options::Impl
    {
        explicit Impl(std::string usage)
            : usage(std::move(usage))
            , desc("OPTIONS")
        {
        }

        std::string usage;
        boost::program_options::options_description desc;
        boost::program_options::variables_map vm;
        std::vector<std::tuple<std::string, std::string>> sub;

        template <typename T>
        auto access(const std::string& k)
        {
            try
            {
                return vm[k].as<T>();
            }
            catch (const std::exception& e)
            {
                throw std::out_of_range("[nil][cli][" + k + "] is invalid");
            }
        }
    };

    Options::Options(
        const OptionInfo& info,
        std::string usage,
        const SubNodes& subnodes,
        int argc,
        const char** argv
    )
        : mImpl(std::make_unique<Impl>(std::move(usage)))

    {
        auto options = mImpl->desc.add_options();
        for (const auto& option : info)
        {
            option->apply({options});
        }

        try
        {
            boost::program_options::positional_options_description positional;
            // TODO: allow positional arguments
            positional.add("__nil_cli_pos_args", 0);

            boost::program_options::command_line_parser parser(argc, argv);
            parser                    //
                .options(mImpl->desc) //
                .positional(positional);

            boost::program_options::store(parser.run(), mImpl->vm);
        }
        catch (const std::exception& ex)
        {
            throw std::invalid_argument(std::string("[nil][cli] ") + ex.what());
        }

        for (const auto& node : subnodes)
        {
            mImpl->sub.emplace_back(std::get<0>(node), std::get<1>(node));
        }
    }

    Options::~Options() = default;

    void Options::help(std::ostream& os) const
    {
        if (!mImpl->usage.empty())
        {
            os << mImpl->usage << '\n';
        }

        if (!mImpl->desc.options().empty())
        {
            mImpl->desc.print(os);
            os << '\n';
        }

        if (!mImpl->sub.empty())
        {
            os << "SUBCOMMANDS:\n";
            const auto width = mImpl->desc.get_option_column_width();
            for (const auto& [key, desc] : mImpl->sub)
            {
                os << "  "                      //
                   << std::left                 //
                   << std::setw(int(width - 2)) //
                   << key;
                // iterate per character.
                // add padding when newline is found.
                // expensive but idc since this is not a hot path.
                for (const auto& c : desc)
                {
                    if (c == '\n')
                    {
                        os << std::setw(int(width));
                    }
                    os << c;
                }
                os << '\n';
            }
            os << '\n';
        }
    }

    bool Options::flag(const std::string& lkey) const
    {
        return mImpl->access<bool>(lkey);
    }

    int Options::number(const std::string& lkey) const
    {
        return mImpl->access<int>(lkey);
    }

    std::string Options::param(const std::string& lkey) const
    {
        return mImpl->access<std::string>(lkey);
    }

    std::vector<std::string> Options::params(const std::string& lkey) const
    {
        return mImpl->access<std::vector<std::string>>(lkey);
    }
}
