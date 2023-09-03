#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace nil::cli
{
    struct IOption
    {
        struct Impl;
        virtual ~IOption() = default;
        virtual void apply(const Impl& impl) const = 0;
    };

    using OptionInfo = std::vector<std::shared_ptr<IOption>>;

    class Builder
    {
    private:
        friend struct Options;

    public:
        Builder();

        /**
         * @brief Register a flag option.
         *  during runtime, if flag is not provided, the default value is false.
         *  only expects that user to provide one instance of the flag.
         *
         * @param msg           message for help
         * @param lkey          long key
         * @param skey          short key
         * @return Builder&     self
         */
        Builder& flag(std::string msg, std::string lkey, std::optional<char> skey = {});

        /**
         * @brief Register a counter option
         *  the default value of the counter is zero.
         *  every argument of counter will increment the value by one.
         *  if a value is provided after the argument, that value will added to the current value.
         *
         *               = 0
         *  -a           = 1
         *  -aaa         = 3
         *  -a -a 2 -a 5 = 8
         *
         * @param msg           message for help
         * @param lkey          long key
         * @param skey          short key
         * @return Builder&     self
         */
        Builder& counter(std::string msg, std::string lkey, std::optional<char> skey = {});

        /**
         * @brief Register a param option.
         *  if fallback is not provided, this option is going to be required.
         *  only expects that user to provide one instance of the option.
         *
         * @param msg           message for help
         * @param lkey          long key
         * @param skey          short key
         * @param fallback      default value when not provided
         * @return Builder&     self
         */
        Builder& param(
            std::string msg,
            std::string lkey,
            std::optional<char> skey = {},
            std::optional<std::string> fallback = {}
        );

        /**
         * @brief Register a collection of param option
         *  the default value is an empty collection.
         *
         *                = []
         *  -a aaa -a bbb = ["aaa", "bbb"]
         *  -a aaa bbb    = ["aaa", "bbb"]
         *
         * @param msg           message for help
         * @param lkey          long key
         * @param skey          short key
         * @return Builder&     self
         */
        Builder& params(std::string msg, std::string lkey, std::optional<char> skey = {});

        /**
         * @brief Finalize the OptionInfo.
         *
         * @return OptionInfo
         */
        OptionInfo build() const;

    private:
        OptionInfo mInfo;
    };
}
