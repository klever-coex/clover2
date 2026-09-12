#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_notification/provider/temperature.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace std::chrono_literals;
using clover2_notification::data::event;

class temperature_provider_test : public ::testing::Test {
protected:
    void SetUp() override {
        m_file = std::filesystem::temp_directory_path() /
                 ("clover2_temperature_provider_" +
                  std::to_string(++s_file_index));
        rclcpp::NodeOptions options;
        options.append_parameter_override("providers.temperature.paths",
                                          std::vector<std::string>{m_file});
        options.append_parameter_override("providers.temperature.period", 0.01);
        options.append_parameter_override("providers.temperature.scale", 0.001);
        options.append_parameter_override("providers.temperature.warning_temperature",
                                          70.0);
        options.append_parameter_override("providers.temperature.error_temperature",
                                          80.0);
        options.append_parameter_override("providers.temperature.precision", 1);
        m_node = std::make_shared<clover2_common::lifecycle_node>(
            "temperature_provider_test_" + std::to_string(s_file_index), options);
        m_executor.add_node(m_node->get_node_base_interface());
        m_spin_thread = std::thread([this]() { m_executor.spin(); });
    }

    void TearDown() override {
        m_provider.cleanup();
        m_executor.cancel();
        if (m_spin_thread.joinable()) {
            m_spin_thread.join();
        }
        m_executor.remove_node(m_node->get_node_base_interface());
        std::filesystem::remove(m_file);
    }

    void write_input(const std::string& value) {
        std::ofstream output(m_file);
        ASSERT_TRUE(output);
        output << value << '\n';
    }

    void initialize() {
        m_provider.initialize(
            std::make_shared<clover2_common::node_context>(*m_node),
            [this](const event& value) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_events.push_back(value);
                m_cv.notify_all();
            });
    }

    event wait_for_event() {
        std::unique_lock<std::mutex> lock(m_mutex);
        EXPECT_TRUE(m_cv.wait_for(lock, 2s, [this]() { return !m_events.empty(); }));
        return m_events.front();
    }

    inline static size_t s_file_index{};
    rclcpp::executors::MultiThreadedExecutor m_executor;
    clover2_common::lifecycle_node::SharedPtr m_node;
    clover2_notification::provider::temperature m_provider;
    std::filesystem::path m_file;
    std::thread m_spin_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::vector<event> m_events;
};

TEST_F(temperature_provider_test, emits_scaled_temperature_with_ok_priority) {
    write_input("52375");
    initialize();

    EXPECT_EQ(wait_for_event(), (event{0, "system", "temperature", "52.4"}));
}

TEST_F(temperature_provider_test, emits_warning_and_error_for_configured_thresholds) {
    write_input("75000");
    initialize();
    EXPECT_EQ(wait_for_event(), (event{1, "system", "temperature", "75.0"}));

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.clear();
    }
    write_input("80000");
    EXPECT_EQ(wait_for_event(), (event{2, "system", "temperature", "80.0"}));
}

TEST_F(temperature_provider_test, emits_stale_event_when_input_is_unavailable) {
    initialize();

    EXPECT_EQ(wait_for_event(), (event{3, "system", "temperature", "n/a"}));
}

TEST_F(temperature_provider_test, reinitializes_after_cleanup) {
    write_input("52375");
    initialize();
    EXPECT_EQ(wait_for_event(), (event{0, "system", "temperature", "52.4"}));

    m_provider.cleanup();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.clear();
    }

    write_input("75000");
    initialize();
    EXPECT_EQ(wait_for_event(), (event{1, "system", "temperature", "75.0"}));
}

}  // namespace