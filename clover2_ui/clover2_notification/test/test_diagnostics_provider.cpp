#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_notification/provider/diagnostics.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace std::chrono_literals;
using diagnostic_array = diagnostic_msgs::msg::DiagnosticArray;
using diagnostic_status = diagnostic_msgs::msg::DiagnosticStatus;
using key_value = diagnostic_msgs::msg::KeyValue;
using clover2_notification::data::event;

key_value make_value(const std::string& key, const std::string& value) {
    key_value result;
    result.key = key;
    result.value = value;
    return result;
}

class diagnostics_provider_test : public ::testing::Test {
protected:
    void SetUp() override {
        rclcpp::NodeOptions options;
        options.append_parameter_override("providers.diagnostics.topic",
                                          "/test_diagnostics");
        options.append_parameter_override(
            "providers.diagnostics.ignore_names",
            std::vector<std::string>{"/Aggregation/System/*"});
        m_node = std::make_shared<clover2_common::lifecycle_node>(
            "diagnostics_provider_test", options);
        m_publisher = m_node->create_publisher<diagnostic_array>(
            "/test_diagnostics", rclcpp::QoS(10));
        m_executor.add_node(m_node->get_node_base_interface());
        m_spin_thread = std::thread([this]() { m_executor.spin(); });

        m_provider.initialize(
            std::make_shared<clover2_common::node_context>(*m_node),
            [this](const event& value) {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_events.push_back(value);
                }
                m_cv.notify_all();
            });
    }

    void TearDown() override {
        m_provider.cleanup();
        m_executor.cancel();
        if (m_spin_thread.joinable()) {
            m_spin_thread.join();
        }
        m_executor.remove_node(m_node->get_node_base_interface());
    }

    bool wait_for_events(size_t count) {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, 2s,
                             [this, count]() { return m_events.size() >= count; });
    }

    void publish(const diagnostic_array& message) {
        ASSERT_TRUE(m_publisher->wait_for_all_acked(100ms) ||
                    m_publisher->get_subscription_count() > 0U);
        m_publisher->publish(message);
    }

    rclcpp::executors::MultiThreadedExecutor m_executor;
    clover2_common::lifecycle_node::SharedPtr m_node;
    rclcpp::Publisher<diagnostic_array>::SharedPtr m_publisher;
    clover2_notification::provider::diagnostics m_provider;
    std::thread m_spin_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::vector<event> m_events;
};

TEST_F(diagnostics_provider_test,
       emits_system_display_events_for_ignored_system_diagnostics) {
    diagnostic_status cpu;
    cpu.name = "/Aggregation/System/test/CPU Information";
    cpu.level = diagnostic_status::OK;
    cpu.values.push_back(make_value("CPU Load Average", "37.25"));

    diagnostic_status sensors;
    sensors.name = "/Aggregation/System/test/Sensor Status";
    sensors.level = diagnostic_status::WARN;
    sensors.values.push_back(make_value("Core 0 Temperature", "51.0"));
    sensors.values.push_back(make_value("Core 1 Temperature", "63.5"));

    diagnostic_array message;
    message.status = {cpu, sensors};
    publish(message);

    ASSERT_TRUE(wait_for_events(2));
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT_EQ(m_events.size(), 2U);
    EXPECT_EQ(m_events[0], (event{0, "system", "cpu", "37.25"}));
    EXPECT_EQ(m_events[1], (event{1, "system", "temperature", "63.5"}));
}

TEST_F(diagnostics_provider_test, reinitializes_after_cleanup) {
    m_provider.cleanup();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.clear();
    }

    m_provider.initialize(
        std::make_shared<clover2_common::node_context>(*m_node),
        [this](const event& value) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_events.push_back(value);
            }
            m_cv.notify_all();
        });

    diagnostic_status status;
    status.name = "/test/diagnostic";
    status.level = diagnostic_status::WARN;
    status.message = "warning";
    diagnostic_array message;
    message.status = {status};
    publish(message);

    ASSERT_TRUE(wait_for_events(1));
    std::lock_guard<std::mutex> lock(m_mutex);
    EXPECT_EQ(m_events[0], (event{1, "diagnostics", "/test/diagnostic",
                                  "warning"}));
}

}  // namespace