#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "conf.hpp"

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
    public:
        /**
         * @brief Register a flag option.
         *  during runtime, if flag is not provided, the default value is false.
         *  only expects that user to provide one instance of the flag.
         *
         * @param lkey          long key
         * @param options       optional data
         * @return Builder&     self
         */
        Builder& flag(std::string lkey, conf::Flag options = {});

        /**
         * @brief Register a number option
         *  the default value is zero.
         *
         * @param lkey          long key
         * @param options       optional data
         * @return Builder&     self
         */
        Builder& number(std::string lkey, conf::Number options = {});

        /**
         * @brief Register a param option.
         *  if fallback is not provided, this option is going to be required.
         *  only expects that user to provide one instance of the option.
         *
         * @param lkey          long key
         * @param options       optional data
         * @return Builder&     self
         */
        Builder& param(std::string lkey, conf::Param options = {});

        /**
         * @brief Register a collection of param option
         *  the default value is an empty collection.
         *
         *                = []
         *  -a aaa -a bbb = ["aaa", "bbb"]
         *  -a aaa bbb    = ["aaa", "bbb"]
         *
         * @param lkey          long key
         * @param options       optional data
         * @return Builder&     self
         */
        Builder& params(std::string lkey, conf::Params options = {});

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
