#include <cli/Builder.hpp>

#include "common.hpp"

#include <optional>
#include <string>

namespace nil::cli
{
    Builder& Builder::flag(std::string lkey, conf::Flag options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Flag options)
                : mLKey(std::move(lkey))
                , mOptions(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mOptions.skey ? (mLKey + ',' + *mOptions.skey) : mLKey;
                auto value = boost::program_options::value<bool>();
                value->zero_tokens();
                value->default_value(false);
                impl.ex(opt.c_str(), value, mOptions.msg.value_or("").c_str());
            }

        private:
            std::string mLKey;
            conf::Flag mOptions;
        };

        mInfo.push_back(std::make_shared<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    Builder& Builder::number(std::string lkey, conf::Number options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Number options)
                : mLKey(std::move(lkey))
                , mOptions(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mOptions.skey ? (mLKey + ',' + *mOptions.skey) : mLKey;
                auto value = boost::program_options::value<int>();
                value->value_name("value");
                value->implicit_value(mOptions.implicit, std::to_string(mOptions.implicit));
                value->default_value(mOptions.fallback, std::to_string(mOptions.fallback));
                impl.ex(opt.c_str(), value, mOptions.msg.value_or("").c_str());
            }

        private:
            std::string mLKey;
            conf::Number mOptions;
        };

        mInfo.push_back(std::make_shared<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    Builder& Builder::param(std::string lkey, conf::Param options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Param options)
                : mLKey(std::move(lkey))
                , mOptions(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mOptions.skey ? (mLKey + ',' + *mOptions.skey) : mLKey;
                auto value = boost::program_options::value<std::string>();
                value->value_name("text");
                if (mOptions.fallback.has_value())
                {
                    value->default_value(
                        mOptions.fallback.value(),
                        "\"" + mOptions.fallback.value() + "\""
                    );
                }
                else
                {
                    value->required();
                }
                impl.ex(opt.c_str(), value, mOptions.msg.value_or("").c_str());
            }

        private:
            std::string mLKey;
            conf::Param mOptions;
        };

        mInfo.push_back(std::make_shared<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    Builder& Builder::params(std::string lkey, conf::Params options)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string lkey, conf::Params options)
                : mLKey(std::move(lkey))
                , mOptions(std::move(options))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mOptions.skey ? (mLKey + ',' + *mOptions.skey) : mLKey;
                auto value = boost::program_options::value<std::vector<std::string>>();
                value->value_name("text");
                value->multitoken();
                value->default_value({}, "");
                impl.ex(opt.c_str(), value, mOptions.msg.value_or("").c_str());
            }

        private:
            std::string mLKey;
            conf::Params mOptions;
        };

        mInfo.push_back(std::make_shared<Option>(std::move(lkey), std::move(options)));
        return *this;
    }

    OptionInfo Builder::build() const
    {
        return std::move(mInfo);
    }
}
