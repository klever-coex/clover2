#include <clover2_http_plugins/utils/msg_json.hpp>

#include <clover2_http_plugins/msg/json_test.hpp>
#include <clover2_http_plugins/msg/json_test_inner.hpp>

#include <gtest/gtest.h>

namespace {

using clover2_http_plugins::msg::JsonTest;
using clover2_http_plugins::msg::JsonTestInner;
namespace msg_json = clover2_http_plugins::utils::msg_json;

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

TEST(MsgJson, GoldenJson) {
    const auto j = msg_json::to_json(make_message());

    EXPECT_EQ(j.at("flag"), true);
    EXPECT_EQ(j.at("byte_value"), 42);
    EXPECT_EQ(j.at("number"), -7);
    EXPECT_EQ(j.at("ratio"), 3.25);
    EXPECT_EQ(j.at("name"), "clover");
    EXPECT_EQ(j.at("fixed_array"), (nlohmann::json::array({1, 2, 3})));
    EXPECT_EQ(j.at("values"), (nlohmann::json::array({0.5, -1.25, 2.0})));
    EXPECT_EQ(j.at("tags"), (nlohmann::json::array({"a", "bb", "ccc"})));
    EXPECT_EQ(j.at("inner").at("text"), "inner-text");
    EXPECT_EQ(j.at("inner").at("count"), 99);
    ASSERT_EQ(j.at("inners").size(), 2u);
    EXPECT_EQ(j.at("inners")[0].at("text"), "first");
    EXPECT_EQ(j.at("inners")[1].at("count"), 2);
}

TEST(MsgJson, RoundTrip) {
    const auto original = make_message();
    const auto j = msg_json::to_json(original);

    JsonTest restored;
    msg_json::from_json(j, restored);

    EXPECT_EQ(restored.flag, original.flag);
    EXPECT_EQ(restored.byte_value, original.byte_value);
    EXPECT_EQ(restored.number, original.number);
    EXPECT_EQ(restored.ratio, original.ratio);
    EXPECT_EQ(restored.name, original.name);
    EXPECT_EQ(restored.fixed_array, original.fixed_array);
    EXPECT_EQ(restored.values, original.values);
    EXPECT_EQ(restored.tags, original.tags);
    EXPECT_EQ(restored.inner.text, original.inner.text);
    EXPECT_EQ(restored.inner.count, original.inner.count);
    ASSERT_EQ(restored.inners.size(), original.inners.size());
    EXPECT_EQ(restored.inners[0].text, original.inners[0].text);
    EXPECT_EQ(restored.inners[1].count, original.inners[1].count);
}

TEST(MsgJson, FromJsonIntoNonEmptyResizesSequences) {
    JsonTest msg = make_message();  // sequences pre-filled
    auto j = msg_json::to_json(make_message());
    j["values"] = nlohmann::json::array({1.0});
    j["tags"] = nlohmann::json::array({"only-one"});

    msg_json::from_json(j, msg);

    ASSERT_EQ(msg.values.size(), 1u);
    EXPECT_EQ(msg.values[0], 1.0);
    ASSERT_EQ(msg.tags.size(), 1u);
    EXPECT_EQ(msg.tags[0], "only-one");
}

TEST(MsgJson, EmptyJsonFails) {
    JsonTest msg;
    EXPECT_THROW(msg_json::from_json(nlohmann::json::object(), msg),
                 std::runtime_error);
}

TEST(MsgJson, FixedArraySizeMismatchFails) {
    JsonTest msg;
    auto j = msg_json::to_json(make_message());
    j["fixed_array"] = nlohmann::json::array({1, 2});

    EXPECT_THROW(msg_json::from_json(j, msg), std::runtime_error);
}

TEST(MsgJson, UnicodeRoundTrip) {
    JsonTestInner msg;
    msg.text = "привет 🚀";
    msg.count = 5;

    const auto j = msg_json::to_json(msg);
    EXPECT_EQ(j.at("text"), "привет 🚀");

    JsonTestInner restored;
    msg_json::from_json(j, restored);
    EXPECT_EQ(restored.text, msg.text);
    EXPECT_EQ(restored.count, msg.count);
}

}  // namespace
