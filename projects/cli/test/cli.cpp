#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nil/cli.hpp>

namespace
{
    // [TODO] verify if bpo changes width depending on terminal width
    const auto help
        = "usage"
          "\nOPTIONS:"
          "\n  -f [ --flag ]                         flag msg"
          "\n  -n [ --number ] [=value(=1)] (=0)     number msg"
          "\n  -p [ --param ] text (=\"default value\")"
          "\n                                        param msg"
          "\n  -m [ --mparam ] text                  mparam msg"
          "\n"
          "\nSUBCOMMANDS:"
          "\n  sub3                                  description of sub3"
          "\n"
          "\n";
}

struct Command final: nil::cli::Command
{
    explicit Command(std::function<int(const nil::cli::Options&)> impl)
        : runImpl(std::move(impl))
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
        return runImpl(options);
    }

    std::function<int(const nil::cli::Options&)> runImpl;
};

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

    auto root = nil::cli::Node::root();
    root.add("sub1", "description of sub1")
        .add<Command>(
            "sub2",
            "description of sub2",
            [&](const nil::cli::Options& options) { return called.Call(options); }
        )
        .add("sub3", "description of sub3");

    const auto matches = [](const nil::cli::Options& output)
    {
        std::ostringstream oss;
        output.help(oss);
        return oss.str() == help &&                     //
            !output.flag("flag") &&                     //
            output.number("number") == 0 &&             //
            output.param("param") == "default value" && //
            output.params("mparam").empty();
    };
    EXPECT_CALL(called, Call(testing::Truly(matches))) //
        .Times(1)                                      //
        .WillOnce(testing::Return(0))
        .RetiresOnSaturation();

    auto args = std::array{"Program", "sub1", "sub2"};
    ASSERT_EQ(root.run(args.size(), args.data()), 0);
}
