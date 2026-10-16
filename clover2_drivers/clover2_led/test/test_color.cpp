#include <clover2_led/data/color.hpp>
#include <clover2_led_msgs/msg/color.hpp>
#include <gtest/gtest.h>

using clover2_led::data::color;

TEST(ColorTest, DefaultConstructorZeroesFields) {
    constexpr color c;
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
}

TEST(ColorTest, ValueConstructorSetsFields) {
    constexpr color c{10, 20, 30};
    EXPECT_EQ(c.r, 10);
    EXPECT_EQ(c.g, 20);
    EXPECT_EQ(c.b, 30);
}

TEST(ColorTest, FromMsgCopiesRGB) {
    clover2_led_msgs::msg::Color msg;
    msg.r = 10;
    msg.g = 20;
    msg.b = 30;

    const color c{msg};
    EXPECT_EQ(c.r, 10);
    EXPECT_EQ(c.g, 20);
    EXPECT_EQ(c.b, 30);
}

TEST(ColorTest, ToMsgRoundtrip) {
    const color c{50, 100, 150};
    const auto msg = c.to_msg();

    EXPECT_EQ(msg.r, 50);
    EXPECT_EQ(msg.g, 100);
    EXPECT_EQ(msg.b, 150);

    const color c2{msg};
    EXPECT_EQ(c2.r, c.r);
    EXPECT_EQ(c2.g, c.g);
    EXPECT_EQ(c2.b, c.b);
}

TEST(ColorTest, EqualColorsCompareEqual) {
    const color a{1, 2, 3};
    const color b{1, 2, 3};
    EXPECT_TRUE(a == b);
}

TEST(ColorTest, DifferentRNotEqual) {
    const color a{1, 2, 3};
    const color b{9, 2, 3};
    EXPECT_FALSE(a == b);
}

TEST(ColorTest, DifferentGNotEqual) {
    const color a{1, 2, 3};
    const color b{1, 9, 3};
    EXPECT_FALSE(a == b);
}

TEST(ColorTest, DifferentBNotEqual) {
    const color a{1, 2, 3};
    const color b{1, 2, 9};
    EXPECT_FALSE(a == b);
}

TEST(ColorTest, DefaultConstructedColorsAreEqual) {
    const color a;
    const color b;
    EXPECT_TRUE(a == b);
}

TEST(ColorTest, MaxUint8Values) {
    constexpr color c{255, 255, 255};
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 255);
}

TEST(ColorTest, CompileTimeConstruction) {
    constexpr color c{42, 43, 44};
    static_assert(c.r == 42);
    static_assert(c.g == 43);
    static_assert(c.b == 44);
}

TEST(ColorTest, FromHUERed) {
    double hue = 0.0;
    double saturation = 1.0;
    double value = 1.0;

    auto c = color::from_hue(hue, saturation, value);
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
}

TEST(ColorTest, FromHUEGreen) {
    double hue = 120.0;
    double saturation = 1.0;
    double value = 1.0;

    auto c = color::from_hue(hue, saturation, value);
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 0);
}

TEST(ColorTest, FromHUEBlue) {
    double hue = 240.0;
    double saturation = 1.0;
    double value = 1.0;

    auto c = color::from_hue(hue, saturation, value);
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 255);
}

TEST(ColorTest, FromHUEBlack) {
    double hue = 0.0;
    double saturation = 0.0;
    double value = 0.0;

    auto c = color::from_hue(hue, saturation, value);
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
}

TEST(ColorTest, FromHUEWhite) {
    double hue = 0.0;
    double saturation = 0.0;
    double value = 1.0;

    auto c = color::from_hue(hue, saturation, value);
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 255);
}
