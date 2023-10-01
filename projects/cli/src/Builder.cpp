#include <nil/cli/Builder.hpp>
#include <nil/cli/types.hpp>

#include "common.hpp"

namespace nil::cli
{
    Builder& Builder::flag(std::string lkey, conf::Flag options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Flag options)
                : lkey(std::move(lkey))
                , options(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = options.skey ? (lkey + ',' + *options.skey) : lkey;
                auto* value = boost::program_options::value<bool>();
                value->zero_tokens();
                value->default_value(false);
                impl.ex(opt.c_str(), value, options.msg.value_or("").c_str());
            }

        private:
            std::string lkey;
            conf::Flag options;
        };

        info.emplace_back(std::make_unique<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    Builder& Builder::number(std::string lkey, conf::Number options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Number options)
                : lkey(std::move(lkey))
                , options(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = options.skey ? (lkey + ',' + *options.skey) : lkey;
                auto* value = boost::program_options::value<std::int64_t>();
                value->value_name("value");
                value->implicit_value(options.implicit, std::to_string(options.implicit));
                value->default_value(options.fallback, std::to_string(options.fallback));
                impl.ex(opt.c_str(), value, options.msg.value_or("").c_str());
            }

        private:
            std::string lkey;
            conf::Number options;
        };

        info.emplace_back(std::make_unique<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    Builder& Builder::param(std::string lkey, conf::Param options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Param options)
                : lkey(std::move(lkey))
                , options(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = options.skey ? (lkey + ',' + *options.skey) : lkey;
                auto* value = boost::program_options::value<std::string>();
                value->value_name("text");
                if (options.fallback.has_value())
                {
                    value->default_value(
                        options.fallback.value(),
                        "\"" + options.fallback.value() + "\""
                    );
                }
                else
                {
                    value->required();
                }
                impl.ex(opt.c_str(), value, options.msg.value_or("").c_str());
            }

        private:
            std::string lkey;
            conf::Param options;
        };

        info.emplace_back(std::make_unique<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    Builder& Builder::params(std::string lkey, conf::Params options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Params options)
                : lkey(std::move(lkey))
                , options(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = options.skey ? (lkey + ',' + *options.skey) : lkey;
                auto* value = boost::program_options::value<std::vector<std::string>>();
                value->value_name("text");
                value->multitoken();
                value->default_value({}, "");
                impl.ex(opt.c_str(), value, options.msg.value_or("").c_str());
            }

        private:
            std::string lkey;
            conf::Params options;
        };

        info.emplace_back(std::make_unique<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    OptionInfo Builder::build()
    {
        return std::move(info);
    }
}
