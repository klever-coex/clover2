#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_display_msgs/srv/get_driver_info.hpp>
#include <clover2_notification/output.hpp>
#include <gtest/gtest.h>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace std::chrono_literals;
using get_driver_info = clover2_display_msgs::srv::GetDriverInfo;

bool is_binary_mono8(const sensor_msgs::msg::Image& image) {
    return std::all_of(
        image.data.begin(), image.data.end(),
        [](const auto pixel) { return pixel == 0 || pixel == 255; });
}

bool has_lit_pixels(const sensor_msgs::msg::Image& image) {
    return std::any_of(image.data.begin(), image.data.end(),
                       [](const auto pixel) { return pixel == 255; });
}

bool has_dark_pixels(const sensor_msgs::msg::Image& image) {
    return std::any_of(image.data.begin(), image.data.end(),
                       [](const auto pixel) { return pixel == 0; });
}

bool are_inverted(const sensor_msgs::msg::Image& lhs,
                  const sensor_msgs::msg::Image& rhs) {
    return lhs.width == rhs.width && lhs.height == rhs.height &&
           lhs.encoding == rhs.encoding && lhs.data.size() == rhs.data.size() &&
           std::equal(lhs.data.begin(), lhs.data.end(), rhs.data.begin(),
                      [](const auto left, const auto right) {
                          return left == static_cast<uint8_t>(255U - right);
                      });
}

class display_output_test : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!rclcpp::ok()) {
            rclcpp::init(0, nullptr);
        }
    }

    static void TearDownTestSuite() { rclcpp::shutdown(); }

    void SetUp() override {
        m_fake_driver = std::make_shared<rclcpp::Node>("fake_display_driver");
        m_info_service = m_fake_driver->create_service<get_driver_info>(
            "/test_display/get_driver_info",
            [](const std::shared_ptr<get_driver_info::Request>,
               std::shared_ptr<get_driver_info::Response> response) {
                response->success = true;
                response->message = "ok";
                response->width = 128;
                response->height = 64;
                response->max_fps = 60.0;
                response->supported_encodings = {"mono8"};
            });
        m_image_sub =
            m_fake_driver->create_subscription<sensor_msgs::msg::Image>(
                "/test_display/image", rclcpp::SystemDefaultsQoS(),
                [this](const sensor_msgs::msg::Image& msg) {
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_images.push_back(msg);
                    }
                    m_cv.notify_all();
                });
        m_notification = std::make_shared<clover2_common::lifecycle_node>(
            "notification_display_test", make_options());
        m_executor.add_node(m_fake_driver);
        m_executor.add_node(m_notification->get_node_base_interface());
        m_spin_thread = std::thread([this]() { m_executor.spin(); });
    }

    void TearDown() override {
        m_executor.cancel();
        if (m_spin_thread.joinable()) {
            m_spin_thread.join();
        }
        m_executor.remove_node(m_notification->get_node_base_interface());
        m_executor.remove_node(m_fake_driver);
    }

    rclcpp::NodeOptions make_options() const {
        rclcpp::NodeOptions options;
        options.append_parameter_override("display.base_path", "test_display");
        options.append_parameter_override("display.refresh_period", 10.0);
        options.append_parameter_override(
            "display.notification_overlay.enabled", true);
        options.append_parameter_override(
            "display.notification_overlay.duration", 0.1);
        options.append_parameter_override(
            "display.status_names",
            std::vector<std::string>{"cpu", "temperature", "network"});
        options.append_parameter_override("display.statuses.cpu.source",
                                          "system");
        options.append_parameter_override("display.statuses.cpu.event_name",
                                          "cpu");
        options.append_parameter_override("display.statuses.cpu.label", "cpu");
        options.append_parameter_override("display.statuses.temperature.source",
                                          "system");
        options.append_parameter_override(
            "display.statuses.temperature.event_name", "temperature");
        options.append_parameter_override("display.statuses.temperature.label",
                                          "temp");
        options.append_parameter_override("display.statuses.network.source",
                                          "system");
        options.append_parameter_override("display.statuses.network.event_name",
                                          "network");
        options.append_parameter_override("display.statuses.network.label",
                                          "net");
        options.append_parameter_override("display.alert.enabled", true);
        options.append_parameter_override("display.alert.invert_period", 0.1);
        return options;
    }

    bool wait_for_images(size_t count,
                         const std::chrono::milliseconds timeout = 2s) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [this, count]() {
            return m_images.size() >= count;
        });
    }

    bool wait_for_image_change(const sensor_msgs::msg::Image& initial_image,
                               const std::chrono::milliseconds timeout = 2s) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [this, &initial_image]() {
            return !m_images.empty() &&
                   m_images.back().data != initial_image.data;
        });
    }

    bool wait_for_inverted_image(
        const sensor_msgs::msg::Image& reference_image,
        const std::chrono::milliseconds timeout = 2s) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [this, &reference_image]() {
            return !m_images.empty() &&
                   are_inverted(m_images.back(), reference_image);
        });
    }

    bool wait_for_matching_image(
        const sensor_msgs::msg::Image& reference_image,
        const std::chrono::milliseconds timeout = 2s) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [this, &reference_image]() {
            return !m_images.empty() && m_images.back().data == reference_image.data;
        });
    }

    std::shared_ptr<clover2_common::node_context> make_context() const {
        return std::make_shared<clover2_common::node_context>(*m_notification);
    }

    rclcpp::executors::MultiThreadedExecutor m_executor;
    rclcpp::Node::SharedPtr m_fake_driver;
    clover2_common::lifecycle_node::SharedPtr m_notification;
    rclcpp::Service<get_driver_info>::SharedPtr m_info_service;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;
    std::thread m_spin_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::vector<sensor_msgs::msg::Image> m_images;
    pluginlib::ClassLoader<clover2_notification::output> m_output_loader{
        "clover2_notification", "clover2_notification::output"};
};

TEST_F(display_output_test, publishes_status_and_notification_overlay_images) {
    auto output = m_output_loader.createSharedInstance("display");
    output->initialize(make_context(), "display");

    ASSERT_TRUE(wait_for_images(1));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        EXPECT_EQ(m_images.back().width, 128U);
        EXPECT_EQ(m_images.back().height, 64U);
        EXPECT_EQ(m_images.back().encoding, "mono8");
        EXPECT_TRUE(is_binary_mono8(m_images.back()));
        EXPECT_TRUE(has_lit_pixels(m_images.back()));
        EXPECT_TRUE(has_dark_pixels(m_images.back()));
    }

    output->push2queue({1, "system", "Network", "wlan0 192.168.1.10"});

    ASSERT_TRUE(wait_for_images(2));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        EXPECT_EQ(m_images.back().width, 128U);
        EXPECT_EQ(m_images.back().height, 64U);
        EXPECT_EQ(m_images.back().encoding, "mono8");
        EXPECT_FALSE(m_images.back().data.empty());
        EXPECT_TRUE(is_binary_mono8(m_images.back()));
        EXPECT_TRUE(has_lit_pixels(m_images.back()));
        EXPECT_TRUE(has_dark_pixels(m_images.back()));
    }

    output->clear();
    output.reset();
}

TEST_F(display_output_test, redraws_status_when_system_status_events_arrive) {
    auto output = m_output_loader.createSharedInstance("display");
    output->initialize(make_context(), "display");

    ASSERT_TRUE(wait_for_images(1));
    sensor_msgs::msg::Image initial_image;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        initial_image = m_images.back();
    }

    output->push2queue({0, "system", "cpu", "37.0"});
    output->push2queue({0, "system", "temperature", "52.0"});
    output->push2queue({0, "system", "network", "wlan0 192.168.1.10"});

    ASSERT_TRUE(wait_for_image_change(initial_image));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        EXPECT_TRUE(is_binary_mono8(m_images.back()));
        EXPECT_NE(m_images.back().data, initial_image.data);
    }

    output->clear();
    output.reset();
}

TEST_F(display_output_test,
       inverts_screen_while_any_configured_status_has_nonzero_priority) {
    auto output = m_output_loader.createSharedInstance("display");
    output->initialize(make_context(), "display");

    ASSERT_TRUE(wait_for_images(1));

    output->push2queue({1, "system", "cpu", "95.0"});

    ASSERT_TRUE(wait_for_images(2));
    sensor_msgs::msg::Image alert_normal_image;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        alert_normal_image = m_images.back();
    }

    ASSERT_TRUE(wait_for_inverted_image(alert_normal_image));

    output->push2queue({0, "system", "cpu", "95.0"});

    ASSERT_TRUE(wait_for_matching_image(alert_normal_image));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        EXPECT_TRUE(is_binary_mono8(m_images.back()));
        EXPECT_FALSE(are_inverted(m_images.back(), alert_normal_image));
    }

    output->clear();
    output.reset();
}

}  // namespace
