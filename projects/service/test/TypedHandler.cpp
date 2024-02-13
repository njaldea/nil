#include <nil/service/TypedHandler.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

template <typename T>
using SMF = testing::StrictMock<testing::MockFunction<void(const std::string&, const T&)>>;

TEST(TypedHandler, basic)
{
    SMF<std::string> mock_with_str;
    SMF<std::uint64_t> mock_with_uint64;

    nil::service::TypedHandler<int> sut;
    sut.add(
        0x0A0B0C0D,
        [&](const std::string& id, const std::string& data) { mock_with_str.Call(id, data); }
    );
    sut.add(
        0x0D0C0B0A,
        [&](const std::string& id, const std::uint64_t& data) { mock_with_uint64.Call(id, data); }
    );

    {
        EXPECT_CALL(mock_with_str, Call("id for str", "hello world")) //
            .Times(1)
            .RetiresOnSaturation();

        sut("id for str",
            "\x0A\x0B\x0C\x0D"
            "hello world",
            15);
    }

    {
        EXPECT_CALL(mock_with_uint64, Call("id for int", 0x0102030405060708)) //
            .Times(1)
            .RetiresOnSaturation();

        sut("id for int",
            "\x0D\x0C\x0B\x0A"
            "\x01\x02\x03\x04\x05\x06\x07\x08",
            12);
    }
}
