#include <cli/Builder.hpp>

#include <boost/program_options/options_description.hpp>

#include <optional>
#include <string>

namespace nil::cli
{
    struct IOption::Impl
    {
        boost::program_options::options_description_easy_init& ex;
    };

    template <typename T, typename... Args>
    auto make_ptr(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    Builder::Builder()
        : mInfo{}
    {
    }

    Builder& Builder::flag(std::string msg, std::string lkey, std::optional<char> skey)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string msg, std::string lkey, std::optional<char> skey)
                : mMsg(std::move(msg))
                , mLKey(std::move(lkey))
                , mSKey(std::move(skey))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mSKey ? (mLKey + ',' + *mSKey) : mLKey;
                impl.ex(
                    opt.c_str(),
                    boost::program_options::value<bool>()->zero_tokens()->default_value(false),
                    mMsg.c_str()
                );
            }

        private:
            std::string mMsg;
            std::string mLKey;
            std::optional<char> mSKey;
        };

        mInfo.push_back(make_ptr<Option>(std::move(msg), std::move(lkey), std::move(skey)));
        return *this;
    }

    Builder& Builder::counter(std::string msg, std::string lkey, std::optional<char> skey)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string msg, std::string lkey, std::optional<char> skey)
                : mMsg(std::move(msg))
                , mLKey(std::move(lkey))
                , mSKey(std::move(skey))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mSKey ? (mLKey + ',' + *mSKey) : mLKey;
                auto value = boost::program_options::value<std::vector<std::size_t>>();
                value->implicit_value({1u}, "");
                value->default_value({}, "0");
                impl.ex(opt.c_str(), value, mMsg.c_str());
            }

        private:
            std::string mMsg;
            std::string mLKey;
            std::optional<char> mSKey;
        };

        mInfo.push_back(make_ptr<Option>(std::move(msg), std::move(lkey), std::move(skey)));
        return *this;
    }

    Builder& Builder::param(
        std::string msg,
        std::string lkey,
        std::optional<char> skey,
        std::optional<std::string> fallback
    )
    {
        class Option final: public IOption
        {
        public:
            Option(
                std::string msg,
                std::string lkey,
                std::optional<char> skey,
                std::optional<std::string> fallback
            )
                : mMsg(std::move(msg))
                , mLKey(std::move(lkey))
                , mSKey(std::move(skey))
                , mFallback(std::move(fallback))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mSKey ? (mLKey + ',' + *mSKey) : mLKey;
                auto value = boost::program_options::value<std::string>();
                if (mFallback.has_value())
                {
                    value->default_value({mFallback.value()}, "\"" + mFallback.value() + "\"");
                }
                else
                {
                    value->required();
                }
                impl.ex(opt.c_str(), value, mMsg.c_str());
            }

        private:
            std::string mMsg;
            std::string mLKey;
            std::optional<char> mSKey;
            std::optional<std::string> mImplicit;
            std::optional<std::string> mFallback;
        };

        mInfo.push_back(
            make_ptr<Option>(std::move(msg), std::move(lkey), std::move(skey), std::move(fallback))
        );
        return *this;
    }

    Builder& Builder::params(std::string msg, std::string lkey, std::optional<char> skey)
    {
        class Option final: public IOption
        {
        public:
            Option(std::string msg, std::string lkey, std::optional<char> skey)
                : mMsg(std::move(msg))
                , mLKey(std::move(lkey))
                , mSKey(std::move(skey))
            {
            }

            void apply(const IOption::Impl& impl) const override
            {
                const auto opt = mSKey ? (mLKey + ',' + *mSKey) : mLKey;
                auto value = boost::program_options::value<std::vector<std::string>>();
                value->multitoken();
                value->default_value({}, "");
                impl.ex(opt.c_str(), value, mMsg.c_str());
            }

        private:
            std::string mMsg;
            std::string mLKey;
            std::optional<char> mSKey;
        };

        mInfo.push_back(make_ptr<Option>(std::move(msg), std::move(lkey), std::move(skey)));
        return *this;
    }

    OptionInfo Builder::build() const
    {
        return std::move(mInfo);
    }
}
