#include <nil/cli.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
    auto help(const std::string& c)
    {
        return //
        "usage"
        "\nOPTIONS:"
        "\n  -f [ --flag ]                         flag msg"
        "\n  -n [ --number ] [=value(=1)] (=0)     number msg"
        "\n  -p [ --param ] text (=\"default value\")"
        "\n                                        param msg"
        "\n  -m [ --mparam ] text                  mparam msg"
        "\n"
        "\nSUBCOMMANDS:"
        "\n  sub" + c + "                                  description of sub" + c +
        "\n"
        "\n";
    }

    struct Command final: nil::cli::Command
    {
        explicit Command(std::function<int(const nil::cli::Options&)> init_impl)
            : impl(std::move(init_impl))
        {
        }

        std::string usage() const override
        {
            return "usage";
        }

        nil::cli::OptionInfo options() const override
        {
            return nil::cli::Builder()
                .flag("flag", {.skey = 'f', .msg = "flag msg"})
                .number("number", {.skey = 'n', .msg = "number msg"})
                .param("param", {.skey = 'p', .msg = "param msg", .fallback = "default value"})
                .params("mparam", {.skey = 'm', .msg = "mparam msg"})
                .build();
        }

        int run(const nil::cli::Options& options) const override
        {
            return impl(options);
        }

        std::function<int(const nil::cli::Options&)> impl;
    };
}

TEST(cli, base)
{
    auto args = std::array{"Program"};
    ASSERT_EQ(nil::cli::Node::root().run(args.size(), args.data()), 0);
}

TEST(cli, depth_one)
{
    testing::StrictMock<testing::MockFunction<int(const nil::cli::Options&)>> called;

    auto root = nil::cli::Node::root<Command>( //
        [&](const nil::cli::Options& options) { return called.Call(options); }
    );

    const auto matches = [](const nil::cli::Options& output)
    {
        return !output.flag("flag") &&                  //
            output.number("number") == 0 &&             //
            output.param("param") == "default value" && //
            output.params("mparam").empty();
    };
    EXPECT_CALL(called, Call(testing::Truly(matches))) //
        .Times(1)                                      //
        .WillOnce(testing::Return(0))
        .RetiresOnSaturation();

    auto args = std::array{"Program"};
    ASSERT_EQ(root.run(args.size(), args.data()), 0);
}

TEST(cli, depth_deep)
{
    testing::StrictMock<testing::MockFunction<int(const nil::cli::Options&)>> called;

    const auto callback = [&](const nil::cli::Options& options) { return called.Call(options); };

    auto root = nil::cli::Node::root<Command>(callback);
    root.add<Command>("sub1", "description of sub1", callback)
        .add<Command>("sub2", "description of sub2", callback)
        .add<Command>("sub3", "description of sub3", callback);

    const auto matches = [](std::string h)
    {
        return [h = std::move(h)](const nil::cli::Options& output)
        {
            std::ostringstream oss;
            output.help(oss);
            return oss.str() == help(h) &&                  //
                !output.flag("flag") &&                     //
                output.number("number") == 0 &&             //
                output.param("param") == "default value" && //
                output.params("mparam").empty();
        };
    };
    {
        EXPECT_CALL(called, Call(testing::Truly(matches("1")))) //
            .Times(1)                                           //
            .WillOnce(testing::Return(0))
            .RetiresOnSaturation();

        auto args = std::array{"Program"};
        ASSERT_EQ(root.run(args.size(), args.data()), 0);
    }
    {
        EXPECT_CALL(called, Call(testing::Truly(matches("2")))) //
            .Times(1)                                           //
            .WillOnce(testing::Return(0))
            .RetiresOnSaturation();

        auto args = std::array{"Program", "sub1"};
        ASSERT_EQ(root.run(args.size(), args.data()), 0);
    }
    {
        EXPECT_CALL(called, Call(testing::Truly(matches("3")))) //
            .Times(1)                                           //
            .WillOnce(testing::Return(0))
            .RetiresOnSaturation();

        auto args = std::array{"Program", "sub1", "sub2"};
        ASSERT_EQ(root.run(args.size(), args.data()), 0);
    }
}
