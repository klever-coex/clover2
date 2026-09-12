#include <clover2_display_msgs/srv/get_driver_info.hpp>
#include <clover2_notification/controller.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace {

using namespace std::chrono_literals;
using get_driver_info = clover2_display_msgs::srv::GetDriverInfo;

class controller_test : public ::testing::Test {
protected:
    void SetUp() override {
        m_fake_display = std::make_shared<rclcpp::Node>("fake_controller_display");
        m_info_service = m_fake_display->create_service<get_driver_info>(
            "/test_controller_display/get_driver_info",
            [](const std::shared_ptr<get_driver_info::Request>,
               std::shared_ptr<get_driver_info::Response> response) {
                response->success = true;
                response->message = "ok";
                response->width = 128;
                response->height = 64;
                response->max_fps = 60.0;
                response->supported_encodings = {"mono8"};
            });
        m_image_sub = m_fake_display->create_subscription<sensor_msgs::msg::Image>(
            "/test_controller_display/image", rclcpp::SystemDefaultsQoS(),
            [this](const sensor_msgs::msg::Image&) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_received_image = true;
                m_cv.notify_all();
            });
    }

    void TearDown() override {
        m_executor.cancel();
        if (m_spin_thread.joinable()) {
            m_spin_thread.join();
        }
        if (m_controller) {
            m_executor.remove_node(m_controller);
        }
        m_executor.remove_node(m_fake_display);
    }

    bool wait_for_image() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, 2s, [this]() { return m_received_image; });
    }

    rclcpp::executors::MultiThreadedExecutor m_executor;
    rclcpp::Node::SharedPtr m_fake_display;
    rclcpp::Service<get_driver_info>::SharedPtr m_info_service;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;
    clover2_notification::controller::SharedPtr m_controller;
    std::thread m_spin_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_received_image{false};
};

TEST_F(controller_test, continues_loading_outputs_after_an_output_fails) {
    rclcpp::NodeOptions options;
    options.append_parameter_override("providers_list", std::vector<std::string>{});
    options.append_parameter_override(
        "output_plugins", std::vector<std::string>{"led_strip", "display"});
    options.append_parameter_override("led_strip.plugin", "missing_output");
    options.append_parameter_override("display.plugin", "display");
    options.append_parameter_override("display.base_path", "test_controller_display");
    options.append_parameter_override("display.refresh_period", 0.01);
    options.append_parameter_override("display.status_names", std::vector<std::string>{});

    m_controller = std::make_shared<clover2_notification::controller>(options);
    m_executor.add_node(m_fake_display);
    m_executor.add_node(m_controller);
    m_spin_thread = std::thread([this]() { m_executor.spin(); });

    EXPECT_TRUE(wait_for_image());
}

}  // namespace