#include <clover2_http/msg/json_test.hpp>
#include <clover2_http/msg/json_test_inner.hpp>
#include <clover2_http/plugins/utils/message_type.hpp>
#include <clover2_http/plugins/utils/msg_json.hpp>

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

namespace {

using clover2_http::msg::JsonTest;
namespace msg_json = clover2_http::plugins::utils::msg_json;
using clover2_http::plugins::utils::message_type;

class TypesupportTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* existing = std::getenv("AMENT_PREFIX_PATH");
        const std::string updated =
            std::string(AMENT_PREFIX) +
            (existing ? std::string(":") + existing : std::string{});
        setenv("AMENT_PREFIX_PATH", updated.c_str(), 1);
    }
};

JsonTest make_message() {
    JsonTest msg;
    msg.flag = true;
    msg.byte_value = 42;
    msg.number = -7;
    msg.ratio = 3.25;
    msg.name = "clover";
    msg.fixed_array = {1, 2, 3};
    msg.values = {0.5, -1.25, 2.0};
    msg.tags = {"a", "bb", "ccc"};
    msg.inner.text = "inner-text";
    msg.inner.count = 99;
    msg.inners.resize(2);
    msg.inners[0].text = "first";
    msg.inners[0].count = 1;
    msg.inners[1].text = "second";
    msg.inners[1].count = 2;
    return msg;
}

TEST_F(TypesupportTest, SerializeDeserializeRoundTrip) {
    message_type type("clover2_http/msg/JsonTest");

    const auto expected = msg_json::to_json(make_message());

    void* msg = type.allocate();
    msg_json::detail::from_json(type.members(), msg, expected);

    rclcpp::SerializedMessage serialized;
    ASSERT_TRUE(type.serialize(msg, serialized));

    void* restored = type.allocate();
    ASSERT_TRUE(type.deserialize(serialized, restored));
    const auto actual = msg_json::detail::to_json(type.members(), restored);

    EXPECT_EQ(actual, expected);

    type.deallocate(restored);
    type.deallocate(msg);
}

TEST_F(TypesupportTest, ReusedBuffer) {
    message_type type("clover2_http/msg/JsonTest");
    void* msg = type.allocate();

    JsonTest first = make_message();
    auto j_first = msg_json::to_json(first);
    msg_json::detail::from_json(type.members(), msg, j_first);
    rclcpp::SerializedMessage s1;
    ASSERT_TRUE(type.serialize(msg, s1));

    JsonTest second = make_message();
    second.values = {1.0};
    second.tags = {"only-one"};
    second.inners.resize(1);
    auto j_second = msg_json::to_json(second);
    msg_json::detail::from_json(type.members(), msg, j_second);
    rclcpp::SerializedMessage s2;
    ASSERT_TRUE(type.serialize(msg, s2));

    void* restored = type.allocate();
    ASSERT_TRUE(type.deserialize(s2, restored));
    EXPECT_EQ(msg_json::detail::to_json(type.members(), restored), j_second);

    type.deallocate(restored);
    type.deallocate(msg);
}

TEST_F(TypesupportTest, UnknownTypeThrows) {
    EXPECT_THROW(
        { message_type type("no_such_pkg/msg/DoesNotExist"); },
        std::runtime_error);
}

}  // namespace
