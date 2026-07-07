#include <clover2_led/data/color.hpp>
#include <clover2_led/data/led_frame.hpp>
#include <gtest/gtest.h>

using clover2_led::data::color;
using clover2_led::data::led_frame;

TEST(LedFrameTest, DefaultConstructionEmpty) {
    const led_frame frame;
    EXPECT_TRUE(frame.empty());
    EXPECT_EQ(frame.size(), 0u);
    EXPECT_FLOAT_EQ(frame.brightness, 1.0f);
}

TEST(LedFrameTest, SizeMatchesPixelCount) {
    led_frame frame;
    frame.pixels = {{0, 0, 0}, {255, 0, 0}, {0, 255, 0}};
    EXPECT_EQ(frame.size(), 3u);
    EXPECT_FALSE(frame.empty());
}

TEST(LedFrameTest, FromMsgCopiesFrame) {
    clover2_led::msg::Color c;
    c.r = 10;
    c.g = 20;
    c.b = 30;

    clover2_led::msg::LedFrame msg;
    msg.colors.push_back(c);
    msg.brightness = 0.5;

    const led_frame f{msg};
    EXPECT_EQ(f[0].r, 10);
    EXPECT_EQ(f[0].g, 20);
    EXPECT_EQ(f[0].b, 30);

    EXPECT_FLOAT_EQ(f.brightness, 0.5f);
}

TEST(LedFrameTest, EmptyAfterClear) {
    led_frame frame;
    frame.pixels = {{1, 2, 3}};
    EXPECT_FALSE(frame.empty());

    frame.pixels.clear();
    EXPECT_TRUE(frame.empty());
    EXPECT_EQ(frame.size(), 0u);
}

TEST(LedFrameTest, MutableSubscriptAccess) {
    led_frame frame;
    frame.pixels = {{10, 20, 30}, {40, 50, 60}};

    frame[0] = color{99, 88, 77};
    EXPECT_EQ(frame[0].r, 99);
    EXPECT_EQ(frame[0].g, 88);
    EXPECT_EQ(frame[0].b, 77);

    EXPECT_EQ(frame[1].r, 40);
    EXPECT_EQ(frame[1].g, 50);
    EXPECT_EQ(frame[1].b, 60);
}

TEST(LedFrameTest, ConstSubscriptAccess) {
    led_frame frame;
    frame.pixels = {color{1, 2, 3}, color{4, 5, 6}};
    const auto& cframe = frame;
    EXPECT_EQ(cframe[0].r, 1);
    EXPECT_EQ(cframe[0].g, 2);
    EXPECT_EQ(cframe[0].b, 3);
    EXPECT_EQ(cframe[1].r, 4);
    EXPECT_EQ(cframe[1].g, 5);
    EXPECT_EQ(cframe[1].b, 6);
}

TEST(LedFrameTest, MutableSubscriptModifiesPixel) {
    led_frame frame;
    frame.pixels = {{0, 0, 0}};

    frame[0].r = 128;
    frame[0].g = 64;
    frame[0].b = 32;

    EXPECT_EQ(frame.pixels[0].r, 128);
    EXPECT_EQ(frame.pixels[0].g, 64);
    EXPECT_EQ(frame.pixels[0].b, 32);
}

TEST(LedFrameTest, BrightnessField) {
    led_frame frame;
    EXPECT_FLOAT_EQ(frame.brightness, 1.0f);

    frame.brightness = 0.5f;
    EXPECT_FLOAT_EQ(frame.brightness, 0.5f);

    frame.brightness = 0.0f;
    EXPECT_FLOAT_EQ(frame.brightness, 0.0f);
}

TEST(LedFrameTest, SinglePixelFrame) {
    led_frame frame;
    frame.pixels = {{255, 255, 255}};

    EXPECT_EQ(frame.size(), 1u);
    EXPECT_FALSE(frame.empty());
}

TEST(LedFrameTest, FilledFabric) {
    led_frame frame = led_frame::filled(color{128, 128, 128}, 2);

    EXPECT_EQ(frame.pixels[0].r, 128);
    EXPECT_EQ(frame.pixels[0].g, 128);
    EXPECT_EQ(frame.pixels[0].b, 128);

    EXPECT_EQ(frame.pixels[1].r, 128);
    EXPECT_EQ(frame.pixels[1].g, 128);
    EXPECT_EQ(frame.pixels[1].b, 128);

    EXPECT_FLOAT_EQ(frame.brightness, 1.f);
}

TEST(LedFrameTest, ToMsgRoundtrip) {
    led_frame frame;
    frame.pixels = {color{1, 2, 3}, color{4, 5, 6}};
    frame.brightness = 0.75f;

    const auto msg = frame.to_msg();

    ASSERT_EQ(msg.colors.size(), 2u);
    EXPECT_EQ(msg.colors[0].r, 1);
    EXPECT_EQ(msg.colors[0].g, 2);
    EXPECT_EQ(msg.colors[0].b, 3);
    EXPECT_EQ(msg.colors[1].r, 4);
    EXPECT_EQ(msg.colors[1].g, 5);
    EXPECT_EQ(msg.colors[1].b, 6);
    EXPECT_FLOAT_EQ(msg.brightness, 0.75f);

    const led_frame restored{msg};
    ASSERT_EQ(restored.size(), 2u);
    EXPECT_EQ(restored[0].r, 1);
    EXPECT_EQ(restored[0].g, 2);
    EXPECT_EQ(restored[0].b, 3);
    EXPECT_EQ(restored[1].r, 4);
    EXPECT_EQ(restored[1].g, 5);
    EXPECT_EQ(restored[1].b, 6);
    EXPECT_FLOAT_EQ(restored.brightness, 0.75f);
}

TEST(LedFrameTest, ToMsgEmptyFrame) {
    const led_frame frame;
    const auto msg = frame.to_msg();

    EXPECT_TRUE(msg.colors.empty());
    EXPECT_FLOAT_EQ(msg.brightness, 1.0f);
}
