#include <cli/Node.hpp>
#include <cli/Options.hpp>

#include "common.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iomanip>
#include <ostream>

namespace nil::cli
{
    struct Options::Impl
    {
        Impl(std::string usage)
            : usage(std::move(usage))
            , desc("OPTIONS")
        {
        }

        std::string usage;
        boost::program_options::options_description desc;
        boost::program_options::variables_map vm;
        std::optional<std::string> sub;
    };

    Options::Options(
        const OptionInfo& info,
        std::string usage,
        const std::vector<std::unique_ptr<Node>>& subnodes,
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
            boost::program_options::notify(mImpl->vm);
        }
        catch (const std::exception& ex)
        {
            throw std::runtime_error(ex.what());
        }

        if (!subnodes.empty())
        {
            std::ostringstream oss;
            oss << "SUBCOMMANDS:\n";
            for (const auto& node : subnodes)
            {
                oss << " >  " << std::left << std::setw(15) //
                    << node->name() << "    "               //
                    << node->description() << "\n";
            }
            mImpl->sub = oss.str();
        }
    }

    Options::~Options() = default;

    void Options::help(std::ostream& os) const
    {
        if (!mImpl->usage.empty())
        {
            os << mImpl->usage << '\n';
        }
        mImpl->desc.print(os);
        if (mImpl->sub)
        {
            os << '\n';
            os << mImpl->sub.value();
            os << '\n';
        }
    }

    std::string Options::param(const std::string& lkey) const
    {
        try
        {
            return mImpl->vm[lkey].as<std::string>();
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("invalid access for key: [" + lkey + "]");
        }
    }

    std::vector<std::string> Options::params(const std::string& lkey) const
    {
        try
        {
            return mImpl->vm[lkey].as<std::vector<std::string>>();
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("invalid access for key: [" + lkey + "]");
        }
    }

    bool Options::flag(const std::string& lkey) const
    {
        try
        {
            return mImpl->vm[lkey].as<bool>();
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("invalid access for key: [" + lkey + "]");
        }
    }

    int Options::number(const std::string& lkey) const
    {
        try
        {
            return mImpl->vm[lkey].as<int>();
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("invalid access for key: [" + lkey + "]");
        }
    }
}
