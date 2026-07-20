#include <clover2_common/node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_led/data/driver_info.hpp>
#include <clover2_led/device/base_device.hpp>
#include <clover2_led/exceptions.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

using clover2_led::data::color;
using clover2_led::data::driver_info;
using clover2_led::data::led_frame;
using clover2_led::device::base_device;

namespace {

uint8_t apply_brightness(uint8_t c, double b) {
    auto t = std::clamp(static_cast<double>(c) * b, 0.0, 255.0);
    return static_cast<uint8_t>(t);
}

class mock_device : public base_device {
public:
    mock_device() = default;

    void set_mock_info(const driver_info& info) { base_device::info() = info; }

    bool initialized{false};
    size_t init_led_count{0};
    bool cleaned_up{false};
    led_frame last_written_frame{};
    bool hardware_brightness_set{false};
    float last_hardware_brightness{0.0f};

protected:
    void on_initialize(size_t led_count) override final {
        initialized = true;
        init_led_count = led_count;
    }

    void on_cleanup() override { cleaned_up = true; }

    void write_raw_frame(const std::vector<color>& colors) override final {
        last_written_frame.pixels = colors;

        if (hardware_brightness_set) {
            for (auto& pixel : last_written_frame.pixels) {
                pixel.r = apply_brightness(pixel.r, last_hardware_brightness);
                pixel.g = apply_brightness(pixel.g, last_hardware_brightness);
                pixel.b = apply_brightness(pixel.b, last_hardware_brightness);
            }
        }
    }

    bool set_hardware_brightness(float brightness) override final {
        hardware_brightness_set = true;
        last_hardware_brightness = brightness;
        return true;
    }
};

class BaseDeviceFixture : public ::testing::Test {
protected:
    void SetUp() override {
        rclcpp::init(0, nullptr);
        m_test_node = std::make_shared<clover2_common::node>("test_node");
        m_node_context =
            std::make_shared<clover2_common::node_context>(*m_test_node);
    }

    void TearDown() override {
        m_node_context.reset();
        m_test_node.reset();
        rclcpp::shutdown();
    }

    void init_device(mock_device& dev, size_t led_count) {
        dev.set_mock_info({led_count, 1e9f});
        dev.initialize("test_device", led_count, m_node_context);
    }

    std::shared_ptr<clover2_common::node> m_test_node;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace

TEST(BaseDeviceTest, InfoStoredAndRetrieved) {
    mock_device dev;
    const driver_info info{64, 60.0f};
    dev.set_mock_info(info);

    const auto& cdev = dev;
    EXPECT_EQ(cdev.info().led_count, 64u);
    EXPECT_FLOAT_EQ(cdev.info().max_fps, 60.0f);
}

TEST(BaseDeviceTest, DefaultBrightnessIsOne) {
    mock_device dev;
    EXPECT_FLOAT_EQ(dev.brightness(), 1.0f);
}

TEST(BaseDeviceTest, SetBrightnessClamped) {
    mock_device dev;

    dev.set_brightness(0.5f);
    EXPECT_FLOAT_EQ(dev.brightness(), 0.5f);

    dev.set_brightness(-0.5f);
    EXPECT_FLOAT_EQ(dev.brightness(), 0.0f);

    dev.set_brightness(2.0f);
    EXPECT_FLOAT_EQ(dev.brightness(), 1.0f);

    EXPECT_TRUE(dev.hardware_brightness_set);
    EXPECT_FLOAT_EQ(dev.last_hardware_brightness, 1.0f);
}

TEST_F(BaseDeviceFixture, InitializeSetsNameAndCallsOnInitialize) {
    mock_device dev;
    EXPECT_FALSE(dev.initialized);

    init_device(dev, 16);
    EXPECT_TRUE(dev.initialized);
    EXPECT_EQ(dev.init_led_count, 16u);
}

TEST_F(BaseDeviceFixture, CleanupCallsOnCleanup) {
    mock_device dev;
    init_device(dev, 1);
    EXPECT_FALSE(dev.cleaned_up);

    dev.cleanup();
    EXPECT_TRUE(dev.cleaned_up);
}

TEST_F(BaseDeviceFixture, WriteThrowsOnMismatchedSize) {
    mock_device dev;
    init_device(dev, 1);

    led_frame frame;
    frame.pixels = {color{1, 2, 3}, color{4, 5, 6}};

    EXPECT_THROW(dev.write(frame), clover2_led::device::frame_size_mismatch);
}

TEST_F(BaseDeviceFixture, WriteForwardsFrame) {
    mock_device dev;
    init_device(dev, 2);

    led_frame frame;
    frame.pixels = {color{255, 128, 64}, color{10, 20, 30}};

    dev.write(frame);

    ASSERT_EQ(dev.last_written_frame.size(), 2u);
    EXPECT_EQ(dev.last_written_frame[0].r, 255);
    EXPECT_EQ(dev.last_written_frame[0].g, 128);
    EXPECT_EQ(dev.last_written_frame[0].b, 64);
    EXPECT_EQ(dev.last_written_frame[1].r, 10);
    EXPECT_EQ(dev.last_written_frame[1].g, 20);
    EXPECT_EQ(dev.last_written_frame[1].b, 30);
}

TEST_F(BaseDeviceFixture, WriteAppliesMasterBrightness) {
    mock_device dev;
    init_device(dev, 1);

    led_frame frame;
    frame.pixels = {color{200, 100, 50}};
    frame.brightness = 0.5f;

    dev.write(frame);

    ASSERT_EQ(dev.last_written_frame.size(), 1u);
    EXPECT_EQ(dev.last_written_frame[0].r, 100);
    EXPECT_EQ(dev.last_written_frame[0].g, 50);
    EXPECT_EQ(dev.last_written_frame[0].b, 25);
}

TEST_F(BaseDeviceFixture, WriteWithZeroBrightnessBlacksOut) {
    mock_device dev;
    init_device(dev, 1);

    led_frame frame;
    frame.pixels = {color{255, 255, 255}};
    frame.brightness = 0.0f;

    dev.write(frame);

    ASSERT_EQ(dev.last_written_frame.size(), 1u);
    EXPECT_EQ(dev.last_written_frame[0].r, 0);
    EXPECT_EQ(dev.last_written_frame[0].g, 0);
    EXPECT_EQ(dev.last_written_frame[0].b, 0);
}
