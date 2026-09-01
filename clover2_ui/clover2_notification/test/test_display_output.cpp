#include <clover2_display_msgs/srv/get_driver_info.hpp>
#include <clover2_notification/output.hpp>
#include <gtest/gtest.h>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <sensor_msgs/msg/image.hpp>

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
        m_image_sub = m_fake_driver->create_subscription<sensor_msgs::msg::Image>(
            "/test_display/image", rclcpp::SystemDefaultsQoS(),
            [this](const sensor_msgs::msg::Image& msg) {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_images.push_back(msg);
                }
                m_cv.notify_all();
            });

        m_notification = std::make_shared<rclcpp_lifecycle::LifecycleNode>(
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
        options.append_parameter_override("display.notification_overlay.enabled",
                                          true);
        options.append_parameter_override(
            "display.notification_overlay.duration", 0.1);
        return options;
    }

    bool wait_for_images(size_t count,
                         const std::chrono::milliseconds timeout = 2s) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout,
                             [this, count]() { return m_images.size() >= count; });
    }

    rclcpp::executors::MultiThreadedExecutor m_executor;
    rclcpp::Node::SharedPtr m_fake_driver;
    rclcpp_lifecycle::LifecycleNode::SharedPtr m_notification;
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
    output->initialize(m_notification, "display");

    ASSERT_TRUE(wait_for_images(1));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        EXPECT_EQ(m_images.back().width, 128U);
        EXPECT_EQ(m_images.back().height, 64U);
        EXPECT_EQ(m_images.back().encoding, "mono8");
    }

    output->push2queue({1, "system", "Network", "wlan0 192.168.1.10"});

    ASSERT_TRUE(wait_for_images(2));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        EXPECT_EQ(m_images.back().width, 128U);
        EXPECT_EQ(m_images.back().height, 64U);
        EXPECT_EQ(m_images.back().encoding, "mono8");
        EXPECT_FALSE(m_images.back().data.empty());
    }

    output->clear();
    output.reset();
}

}  // namespace