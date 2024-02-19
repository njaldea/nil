#include <nil/gate/detail/types.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace test
{
    template <typename... T>
    struct Input
    {
        std::tuple<> operator()(T... /* unused */)
        {
            return {};
        }
    };

    template <typename... T>
    struct Output
    {
        std::tuple<T...> operator()()
        {
            return {};
        }
    };
}

TEST(gate, traits_zero_input)
{
    using type = nil::gate::detail::traits<test::Input<>>;
    ASSERT_EQ(type::inputs::size, 0);
    ASSERT_TRUE((std::is_same_v<type::inputs::edges, std::tuple<>>));
    ASSERT_TRUE((std::is_same_v<type::inputs::readonly_edges, std::tuple<>>));
    ASSERT_TRUE((std::is_same_v<type::inputs::mutable_edges, std::tuple<>>));
}

TEST(gate, traits_zero_output)
{
    using type = nil::gate::detail::traits<test::Output<>>::all_outputs;
    ASSERT_EQ(type::size, 0);
    ASSERT_TRUE((std::is_same_v<type::edges, std::tuple<>>));
    ASSERT_TRUE((std::is_same_v<type::readonly_edges, std::tuple<>>));
    ASSERT_TRUE((std::is_same_v<type::mutable_edges, std::tuple<>>));
}

TEST(gate, traits_void_return)
{
    using Callable = decltype([]() {});

    using type = nil::gate::detail::traits<Callable>::all_outputs;
    ASSERT_EQ(type::size, 0);
    ASSERT_TRUE((std::is_same_v<type::edges, std::tuple<>>));
    ASSERT_TRUE((std::is_same_v<type::readonly_edges, std::tuple<>>));
    ASSERT_TRUE((std::is_same_v<type::mutable_edges, std::tuple<>>));
}

TEST(gate, traits_multiple_return)
{
    using namespace nil::gate::detail;
    using namespace nil::gate;
    using type = traits<test::Output<int, bool, std::string>>::all_outputs;
    ASSERT_EQ(type::size, 3);
    // clang-format off
    ASSERT_TRUE((std::is_same_v<type::edges, std::tuple<DataEdge<int>, DataEdge<bool>, DataEdge<std::string>>>));
    ASSERT_TRUE((std::is_same_v<type::readonly_edges, std::tuple<ReadOnlyEdge<int>*, ReadOnlyEdge<bool>*, ReadOnlyEdge<std::string>*>>));
    ASSERT_TRUE((std::is_same_v<type::mutable_edges, std::tuple<MutableEdge<int>*, MutableEdge<bool>*, MutableEdge<std::string>*>>));
    // clang-format on
}

TEST(gate, traits_multiple_input)
{
    using namespace nil::gate::detail;
    using namespace nil::gate;
    using type = traits<test::Input<int, const int&>>::inputs;
    ASSERT_EQ(type::size, 2);
    // clang-format off
    ASSERT_TRUE((std::is_same_v<type::edges, std::tuple<DataEdge<int>, DataEdge<int>>>));
    ASSERT_TRUE((std::is_same_v<type::readonly_edges, std::tuple<ReadOnlyEdge<int>*, ReadOnlyEdge<int>*>>));
    ASSERT_TRUE((std::is_same_v<type::mutable_edges, std::tuple<MutableEdge<int>*, MutableEdge<int>*>>));
    // clang-format on
}

TEST(gate, traits_input_validity)
{
    using namespace nil::gate::detail;
    ASSERT_TRUE(traits<test::Input<int>>::is_valid);

    ASSERT_FALSE(traits<test::Input<int&>>::is_valid);
    ASSERT_FALSE(traits<test::Input<int*>>::is_valid);

    ASSERT_TRUE(traits<test::Input<const int&>>::is_valid);
    ASSERT_FALSE(traits<test::Input<const int*>>::is_valid);

    ASSERT_FALSE(traits<test::Input<std::unique_ptr<int>>>::is_valid);
    ASSERT_FALSE(traits<test::Input<std::shared_ptr<int>>>::is_valid);

    ASSERT_FALSE(traits<test::Input<std::unique_ptr<int>&>>::is_valid);
    ASSERT_FALSE(traits<test::Input<std::shared_ptr<int>&>>::is_valid);

    ASSERT_FALSE(traits<test::Input<const std::unique_ptr<int>&>>::is_valid);
    ASSERT_FALSE(traits<test::Input<const std::shared_ptr<int>&>>::is_valid);

    ASSERT_FALSE(traits<test::Input<std::unique_ptr<const int>>>::is_valid);
    ASSERT_TRUE(traits<test::Input<std::shared_ptr<const int>>>::is_valid);

    ASSERT_TRUE(traits<test::Input<const std::unique_ptr<const int>&>>::is_valid);
    ASSERT_TRUE(traits<test::Input<const std::shared_ptr<const int>&>>::is_valid);
}

TEST(gate, traits_output_validity)
{
    using namespace nil::gate::detail;
    ASSERT_TRUE(traits<test::Output<int>>::is_valid);

    ASSERT_FALSE(traits<test::Output<int&>>::is_valid);
    ASSERT_FALSE(traits<test::Output<int*>>::is_valid);

    ASSERT_FALSE(traits<test::Output<const int&>>::is_valid);
    ASSERT_FALSE(traits<test::Output<const int*>>::is_valid);

    ASSERT_FALSE(traits<test::Output<std::unique_ptr<int>>>::is_valid);
    ASSERT_FALSE(traits<test::Output<std::shared_ptr<int>>>::is_valid);

    ASSERT_FALSE(traits<test::Output<std::unique_ptr<int>&>>::is_valid);
    ASSERT_FALSE(traits<test::Output<std::unique_ptr<int>&>>::is_valid);

    ASSERT_FALSE(traits<test::Output<const std::unique_ptr<int>&>>::is_valid);
    ASSERT_FALSE(traits<test::Output<const std::shared_ptr<int>&>>::is_valid);

    ASSERT_TRUE(traits<test::Output<std::unique_ptr<const int>>>::is_valid);
    ASSERT_TRUE(traits<test::Output<std::shared_ptr<const int>>>::is_valid);

    ASSERT_FALSE(traits<test::Output<const std::unique_ptr<const int>&>>::is_valid);
    ASSERT_FALSE(traits<test::Output<const std::shared_ptr<const int>&>>::is_valid);
}
