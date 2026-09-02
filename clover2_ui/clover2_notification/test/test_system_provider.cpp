#include <clover2_common/node_context.hpp>
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_notification/provider/system.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/temperature.hpp>
#include <std_msgs/msg/float64.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

class system_provider_test : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!rclcpp::ok()) {
            rclcpp::init(0, nullptr);
        }
    }

    static void TearDownTestSuite() { rclcpp::shutdown(); }

    void SetUp() override {
        m_tmp_dir = std::filesystem::temp_directory_path() /
                    std::filesystem::path("clover2_system_provider_test_" +
                                          std::to_string(++s_counter));
        std::filesystem::remove_all(m_tmp_dir);
        std::filesystem::create_directories(m_tmp_dir);
    }

    void TearDown() override { std::filesystem::remove_all(m_tmp_dir); }

    rclcpp::NodeOptions make_base_options() const {
        rclcpp::NodeOptions options;
        options.append_parameter_override("autostart", false);
        options.append_parameter_override("providers.system.period", 10.0);
        options.append_parameter_override("providers.system.cpu.enabled", false);
        options.append_parameter_override("providers.system.temperature.enabled",
                                          false);
        options.append_parameter_override("providers.system.network.enabled",
                                          false);
        return options;
    }

    std::shared_ptr<clover2_common::node_context> make_context(
        clover2_common::lifecycle_node& node) const {
        return std::make_shared<clover2_common::node_context>(node);
    }

    static void write_file(const std::filesystem::path& path,
                           const std::string& content) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path);
        file << content;
    }

    std::filesystem::path m_tmp_dir;
    static inline int s_counter{};
};

TEST_F(system_provider_test, disabled_provider_does_not_emit_events) {
    auto options = make_base_options();
    options.append_parameter_override("providers.system.enabled", false);

    auto node = std::make_shared<clover2_common::lifecycle_node>(
        "system_provider_disabled_test", options);
    std::vector<clover2_notification::data::event> events;

    clover2_notification::provider::system provider;
    provider.initialize(make_context(*node),
                        [&events](const clover2_notification::data::event& event) {
                            events.push_back(event);
                        });

    EXPECT_TRUE(events.empty());
    provider.cleanup();
}

TEST_F(system_provider_test, emits_temperature_warning_from_fake_thermal_zone) {
    const auto thermal_base = m_tmp_dir / "thermal";
    write_file(thermal_base / "thermal_zone0" / "temp", "75000\n");

    auto options = make_base_options();
    options.append_parameter_override("providers.system.enabled", true);
    options.append_parameter_override("providers.system.temperature.enabled",
                                      true);
    options.append_parameter_override(
        "providers.system.temperature.thermal_zone", "thermal_zone0");
    options.append_parameter_override(
        "providers.system.temperature.warn_celsius", 70.0);
    options.append_parameter_override(
        "providers.system.temperature.error_celsius", 85.0);
    options.append_parameter_override("providers.system.thermal_base_path",
                                      thermal_base.string());

    auto node = std::make_shared<clover2_common::lifecycle_node>(
        "system_provider_temp_test", options);
    std::vector<clover2_notification::data::event> events;
    std::optional<sensor_msgs::msg::Temperature> temperature_message;
    const auto subscription =
        node->create_subscription<sensor_msgs::msg::Temperature>(
            "system/temperature", rclcpp::QoS(1).transient_local(),
            [&temperature_message](const sensor_msgs::msg::Temperature& message) {
                temperature_message = message;
            });

    clover2_notification::provider::system provider;
    provider.initialize(make_context(*node),
                        [&events](const clover2_notification::data::event& event) {
                            events.push_back(event);
                        });

    rclcpp::spin_some(node->get_node_base_interface());

    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0].priority, 1);
    EXPECT_EQ(events[0].source, "system");
    EXPECT_EQ(events[0].name, "CPU temperature");
    EXPECT_NE(events[0].message.find("TEMP 75.0C WARN"), std::string::npos);
    ASSERT_TRUE(temperature_message);
    EXPECT_DOUBLE_EQ(temperature_message->temperature, 75.0);
    EXPECT_EQ(temperature_message->header.frame_id, "cpu");
    provider.cleanup();
}

TEST_F(system_provider_test, emits_network_warning_from_fake_interface) {
    const auto net_base = m_tmp_dir / "net";
    write_file(net_base / "wlan0" / "operstate", "down\n");
    write_file(net_base / "wlan0" / "ipv4_address", "192.168.1.10\n");

    auto options = make_base_options();
    options.append_parameter_override("providers.system.enabled", true);
    options.append_parameter_override("providers.system.network.enabled", true);
    options.append_parameter_override("providers.system.network.interfaces",
                                      std::vector<std::string>{"wlan0"});
    options.append_parameter_override("providers.system.net_base_path",
                                      net_base.string());

    auto node = std::make_shared<clover2_common::lifecycle_node>(
        "system_provider_net_test", options);
    std::vector<clover2_notification::data::event> events;
    std::optional<diagnostic_msgs::msg::DiagnosticArray> network_message;
    const auto subscription =
        node->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
            "system/network", rclcpp::QoS(1).transient_local(),
            [&network_message](const diagnostic_msgs::msg::DiagnosticArray& message) {
                network_message = message;
            });

    clover2_notification::provider::system provider;
    provider.initialize(make_context(*node),
                        [&events](const clover2_notification::data::event& event) {
                            events.push_back(event);
                        });

    rclcpp::spin_some(node->get_node_base_interface());

    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0].priority, 1);
    EXPECT_EQ(events[0].source, "system");
    EXPECT_EQ(events[0].name, "Network interface");
    EXPECT_NE(events[0].message.find("wlan0 192.168.1.10"),
              std::string::npos);
    ASSERT_TRUE(network_message);
    ASSERT_EQ(network_message->status.size(), 1U);
    EXPECT_EQ(network_message->status[0].name, "system/network/wlan0");
    EXPECT_EQ(network_message->status[0].level,
              diagnostic_msgs::msg::DiagnosticStatus::WARN);
    ASSERT_EQ(network_message->status[0].values.size(), 3U);
    EXPECT_EQ(network_message->status[0].values[2].key, "ipv4");
    EXPECT_EQ(network_message->status[0].values[2].value, "192.168.1.10");
    provider.cleanup();
}

TEST_F(system_provider_test, publishes_cpu_usage_from_fake_proc_stat) {
    const auto proc_stat = m_tmp_dir / "stat";
    write_file(proc_stat, "cpu  100 0 100 800 0 0 0 0 0 0\n");

    auto options = make_base_options();
    options.append_parameter_override("providers.system.enabled", true);
    options.append_parameter_override("providers.system.cpu.enabled", true);
    options.append_parameter_override("providers.system.proc_stat_path",
                                      proc_stat.string());

    auto node = std::make_shared<clover2_common::lifecycle_node>(
        "system_provider_cpu_test", options);
    std::optional<std_msgs::msg::Float64> cpu_message;
    const auto subscription = node->create_subscription<std_msgs::msg::Float64>(
        "system/cpu", rclcpp::QoS(1).transient_local(),
        [&cpu_message](const std_msgs::msg::Float64& message) {
            cpu_message = message;
        });

    clover2_notification::provider::system provider;
    provider.initialize(make_context(*node),
                        [](const clover2_notification::data::event&) {});

    rclcpp::spin_some(node->get_node_base_interface());

    ASSERT_TRUE(cpu_message);
    EXPECT_DOUBLE_EQ(cpu_message->data, 0.0);
    provider.cleanup();
}

}  // namespace