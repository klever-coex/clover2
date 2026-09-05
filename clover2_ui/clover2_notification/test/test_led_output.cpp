#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_led_msgs/srv/get_driver_info.hpp>
#include <clover2_led_msgs/srv/start_animation.hpp>
#include <clover2_notification/output.hpp>
#include <gtest/gtest.h>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace {

using namespace std::chrono_literals;
using start_animation = clover2_led_msgs::srv::StartAnimation;

class led_output_test : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!rclcpp::ok()) {
            rclcpp::init(0, nullptr);
        }
    }

    static void TearDownTestSuite() { rclcpp::shutdown(); }

    void SetUp() override {
        m_fake_driver = std::make_shared<rclcpp::Node>("fake_led_driver");
        m_info_service = m_fake_driver->create_service<
            clover2_led_msgs::srv::GetDriverInfo>(
            "/test_led/get_driver_info",
            [](const std::shared_ptr<
                   clover2_led_msgs::srv::GetDriverInfo::Request>,
               std::shared_ptr<clover2_led_msgs::srv::GetDriverInfo::Response>
                   response) {
                response->success = true;
                response->message = "ok";
                response->led_count = 8;
                response->max_fps = 50.0;
            });
        m_start_service = m_fake_driver->create_service<start_animation>(
            "/test_led/start_animation",
            [this](const std::shared_ptr<start_animation::Request> request,
                   std::shared_ptr<start_animation::Response> response) {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_request = *request;
                    m_received = true;
                }
                m_cv.notify_all();
                response->success = true;
                response->message = "ok";
            });

        m_notification = create_notification_node(make_valid_options());
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

    rclcpp::NodeOptions make_base_options() const {
        rclcpp::NodeOptions options;
        options.append_parameter_override("led_strip.base_path", "test_led");
        return options;
    }

    rclcpp::NodeOptions make_valid_options() const {
        auto options = make_base_options();
        options.append_parameter_override(
            "led_strip.reaction_names",
            std::vector<std::string>{"test_warning", "power"});
        options.append_parameter_override(
            "led_strip.reactions.test_warning.event_name", "Test warning");
        options.append_parameter_override(
            "led_strip.reactions.test_warning.animation", "blink");
        options.append_parameter_override(
            "led_strip.reactions.test_warning.color",
            std::vector<int64_t>{255, 255, 0});
        options.append_parameter_override(
            "led_strip.reactions.test_warning.brightness", 0.7);
        options.append_parameter_override(
            "led_strip.reactions.test_warning.period", 1.0);
        options.append_parameter_override(
            "led_strip.reactions.test_warning.duration", 3.0);

        options.append_parameter_override(
            "led_strip.reactions.power.event_name", "Power");
        options.append_parameter_override("led_strip.reactions.power.animation",
                                          "blink");
        options.append_parameter_override("led_strip.reactions.power.color",
                                          std::vector<int64_t>{255, 128, 0});
        options.append_parameter_override(
            "led_strip.reactions.power.brightness", 0.8);
        options.append_parameter_override("led_strip.reactions.power.period",
                                          0.8);
        options.append_parameter_override("led_strip.reactions.power.duration",
                                          3.0);
        return options;
    }

    clover2_common::lifecycle_node::SharedPtr create_notification_node(
        const rclcpp::NodeOptions& options) const {
        return std::make_shared<clover2_common::lifecycle_node>(
            "notification_test", options);
    }

    bool wait_for_request(const std::chrono::milliseconds timeout = 2s) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [this]() { return m_received; });
    }

    std::shared_ptr<clover2_common::node_context> make_context() const {
        return std::make_shared<clover2_common::node_context>(*m_notification);
    }

    rclcpp::executors::MultiThreadedExecutor m_executor;
    rclcpp::Node::SharedPtr m_fake_driver;
    clover2_common::lifecycle_node::SharedPtr m_notification;
    rclcpp::Service<clover2_led_msgs::srv::GetDriverInfo>::SharedPtr
        m_info_service;
    rclcpp::Service<start_animation>::SharedPtr m_start_service;
    std::thread m_spin_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_received{false};
    start_animation::Request m_request;
    pluginlib::ClassLoader<clover2_notification::output> m_output_loader{
        "clover2_notification", "clover2_notification::output"};
};

TEST_F(led_output_test, maps_configured_named_reaction_to_led_animation) {
    auto output = m_output_loader.createSharedInstance("led");
    output->initialize(make_context(), "led_strip");

    output->push2queue({1, "diagnostics", "Test warning", "low battery"});

    ASSERT_TRUE(wait_for_request());
    EXPECT_EQ(m_request.animation_name, "blink");
    EXPECT_FLOAT_EQ(m_request.brightness, 0.7F);
    EXPECT_FLOAT_EQ(m_request.period, 1.0F);
    EXPECT_FLOAT_EQ(m_request.duration, 3.0F);
    ASSERT_EQ(m_request.colors.size(), 1U);
    EXPECT_EQ(m_request.colors[0].r, 255U);
    EXPECT_EQ(m_request.colors[0].g, 255U);
    EXPECT_EQ(m_request.colors[0].b, 0U);
}

TEST_F(led_output_test, ignores_event_without_configured_reaction) {
    auto output = m_output_loader.createSharedInstance("led");
    output->initialize(make_context(), "led_strip");

    output->push2queue({2, "diagnostics", "Test error", "failure"});

    EXPECT_FALSE(wait_for_request(200ms));
}

TEST_F(led_output_test, throws_when_configured_reaction_is_incomplete) {
    m_executor.remove_node(m_notification->get_node_base_interface());
    auto options = make_base_options();
    options.append_parameter_override("led_strip.reaction_names",
                                      std::vector<std::string>{"warn"});
    options.append_parameter_override("led_strip.reactions.warn.event_name",
                                      "Test warning");
    options.append_parameter_override("led_strip.reactions.warn.animation",
                                      "blink");
    options.append_parameter_override("led_strip.reactions.warn.color",
                                      std::vector<int64_t>{255, 255, 0});
    options.append_parameter_override("led_strip.reactions.warn.brightness",
                                      0.7);
    options.append_parameter_override("led_strip.reactions.warn.period", 1.0);
    m_notification = create_notification_node(options);
    m_executor.add_node(m_notification->get_node_base_interface());

    auto output = m_output_loader.createSharedInstance("led");
    EXPECT_THROW(output->initialize(make_context(), "led_strip"),
                 std::runtime_error);
}

TEST_F(led_output_test, throws_when_configured_reaction_without_event_name) {
    m_executor.remove_node(m_notification->get_node_base_interface());
    auto options = make_base_options();
    options.append_parameter_override("led_strip.reaction_names",
                                      std::vector<std::string>{"power"});
    options.append_parameter_override("led_strip.reactions.power.animation",
                                      "blink");
    options.append_parameter_override("led_strip.reactions.power.color",
                                      std::vector<int64_t>{255, 128, 0});
    options.append_parameter_override("led_strip.reactions.power.brightness",
                                      0.8);
    options.append_parameter_override("led_strip.reactions.power.period", 0.8);
    options.append_parameter_override("led_strip.reactions.power.duration",
                                      3.0);
    m_notification = create_notification_node(options);
    m_executor.add_node(m_notification->get_node_base_interface());

    auto output = m_output_loader.createSharedInstance("led");
    EXPECT_THROW(output->initialize(make_context(), "led_strip"),
                 std::runtime_error);
}

TEST_F(led_output_test, maps_configured_power_reaction_to_led_animation) {
    auto output = m_output_loader.createSharedInstance("led");
    output->initialize(make_context(), "led_strip");

    output->push2queue({1, "diagnostics", "Power", "Test warning"});

    ASSERT_TRUE(wait_for_request());
    EXPECT_EQ(m_request.animation_name, "blink");
    EXPECT_FLOAT_EQ(m_request.brightness, 0.8F);
    EXPECT_FLOAT_EQ(m_request.period, 0.8F);
    EXPECT_FLOAT_EQ(m_request.duration, 3.0F);
    ASSERT_EQ(m_request.colors.size(), 1U);
    EXPECT_EQ(m_request.colors[0].r, 255U);
    EXPECT_EQ(m_request.colors[0].g, 128U);
    EXPECT_EQ(m_request.colors[0].b, 0U);
}

}  // namespace
