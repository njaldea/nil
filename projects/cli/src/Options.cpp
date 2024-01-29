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
    struct Options::Impl final
    {
        explicit Impl(std::string init_usage)
            : usage(std::move(init_usage))
            , desc("OPTIONS")
        {
        }

        ~Impl() noexcept = default;

        Impl(Impl&&) = delete;
        Impl(const Impl&) = delete;
        Impl& operator=(Impl&&) = delete;
        Impl& operator=(const Impl&) = delete;

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
            catch (const std::exception&)
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
        const char* const* argv
    )
        : impl(std::make_unique<Impl>(std::move(usage)))
    {
        auto options = impl->desc.add_options();
        for (const auto& option : info)
        {
            option->apply({&options});
        }

        try
        {
            boost::program_options::positional_options_description positional;
            // [TODO] allow positional arguments
            positional.add("__nil_cli_pos_args", 0);

            boost::program_options::command_line_parser parser(argc, argv);
            parser                   //
                .options(impl->desc) //
                .positional(positional);

            boost::program_options::store(parser.run(), impl->vm);
        }
        catch (const std::exception& ex)
        {
            throw std::invalid_argument(std::string("[nil][cli] ") + ex.what());
        }

        for (const auto& node : subnodes)
        {
            impl->sub.emplace_back(std::get<0>(node), std::get<1>(node));
        }
    }

    Options::~Options() noexcept = default;

    void Options::help(std::ostream& os) const
    {
        if (!impl->usage.empty())
        {
            os << impl->usage << '\n';
        }

        if (!impl->desc.options().empty())
        {
            impl->desc.print(os);
            os << '\n';
        }

        if (!impl->sub.empty())
        {
            os << "SUBCOMMANDS:\n";
            const auto width = impl->desc.get_option_column_width();
            for (const auto& [key, desc] : impl->sub)
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
        return impl->access<bool>(lkey);
    }

    std::int64_t Options::number(const std::string& lkey) const
    {
        return impl->access<std::int64_t>(lkey);
    }

    std::string Options::param(const std::string& lkey) const
    {
        return impl->access<std::string>(lkey);
    }

    std::vector<std::string> Options::params(const std::string& lkey) const
    {
        return impl->access<std::vector<std::string>>(lkey);
    }
}
