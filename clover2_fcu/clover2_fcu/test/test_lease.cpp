// clover2
#include <clover2_common/node.hpp>
#include <clover2_common/util/sync_service.hpp>
#include <clover2_fcu/bridge.hpp>
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/capability/setpoint_position_interface.hpp>
#include <clover2_fcu/client.hpp>
#include <clover2_fcu/mode_base.hpp>
#include <clover2_fcu_msgs/msg/mode_state.hpp>
#include <clover2_fcu_msgs/srv/activate_mode.hpp>

#include "stub_backend.hpp"

// ROS2
#include <rclcpp/rclcpp.hpp>

// third party
#include <gtest/gtest.h>

// STL
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

class ros_env : public ::testing::Environment {
public:
    void SetUp() override { rclcpp::init(0, nullptr); }
    void TearDown() override { rclcpp::shutdown(); }
};

[[maybe_unused]] const auto* g_env =
    ::testing::AddGlobalTestEnvironment(new ros_env());

using clover2_common::util::call_sync;
using clover2_fcu_msgs::msg::ModeState;
using clover2_fcu_msgs::srv::ActivateMode;

/// @brief The stub backend is a test-support shared library: pluginlib
///        discovers backends only through installed package XMLs, so the
///        test installs it into a temporary ament prefix (resource index
///        + plugins xml + library copy) and prepends it to the
///        AMENT_PREFIX_PATH the ClassLoaders of bridge/client scan.
class stub_plugin_env {
public:
    void install() {
        m_prefix = std::filesystem::temp_directory_path() /
                   ("fcu_stub_" + std::to_string(::getpid()));

        const auto resource_dir = m_prefix / "share/ament_index/resource_index";
        std::filesystem::create_directories(resource_dir /
                                            "clover2_fcu__pluginlib__plugin");
        std::filesystem::create_directories(resource_dir / "packages");
        std::filesystem::create_directories(m_prefix / "share/test_stub");
        std::filesystem::create_directories(m_prefix / "lib");

        std::filesystem::copy_file(
            STUB_LIBRARY_PATH, m_prefix / "lib/libtest_stub_backend.so",
            std::filesystem::copy_options::overwrite_existing);
        write_file(resource_dir / "clover2_fcu__pluginlib__plugin/test_stub",
                   "share/test_stub/test_stub_plugins.xml\n");
        write_file(resource_dir / "packages/test_stub", "");
        write_file(m_prefix / "share/test_stub/package.xml",
                   R"(<?xml version="1.0"?>
<package format="3">
  <name>test_stub</name>
  <version>0.0.0</version>
  <description>Test stub plugin carrier</description>
  <maintainer email="test@example.com">test</maintainer>
  <license>None</license>
  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>
)");
        write_file(m_prefix / "share/test_stub/test_stub_plugins.xml",
                   R"(<?xml version="1.0"?>
<library path="test_stub_backend">
  <class name="stub"
         type="clover2_fcu::stub::factory"
         base_class_type="clover2_fcu::backend::backend_base">
    <description>Test stub FCU backend</description>
  </class>
</library>
)");

        const char* current = std::getenv("AMENT_PREFIX_PATH");
        m_saved_prefix_path = current ? current : "";
        const std::string updated =
            m_prefix.string() +
            (m_saved_prefix_path.empty() ? "" : ":" + m_saved_prefix_path);
        ::setenv("AMENT_PREFIX_PATH", updated.c_str(), 1);
    }

    void uninstall() {
        ::setenv("AMENT_PREFIX_PATH", m_saved_prefix_path.c_str(), 1);
        std::error_code error;
        std::filesystem::remove_all(m_prefix, error);
    }

private:
    static void write_file(const std::filesystem::path& path,
                           const std::string& content) {
        std::ofstream stream(path);
        stream << content;
    }

    std::filesystem::path m_prefix;
    std::string m_saved_prefix_path;
};

/// @brief Test controller: streams position setpoints and counts ticks.
class test_mode : public clover2_fcu::mode_base {
public:
    using mode_base::mode_base;

    std::atomic<int> updates{0};

protected:
    void update_setpoint(double dt_s) override {
        (void)dt_s;
        ++updates;
        position()->update(Eigen::Vector3d::Zero());
    }
};

/// @brief One bridge (protocol=stub) plus one client node under a shared
///        background executor; the lease model runs end-to-end through
///        the real services and the mode_state topic.
class lease_test : public ::testing::Test {
protected:
    void SetUp() override {
        m_plugin_env.install();

        m_bridge = std::make_shared<clover2_fcu::bridge>(
            rclcpp::NodeOptions().parameter_overrides(
                {{"protocol", rclcpp::ParameterValue("stub")}}));
        m_node = std::make_shared<clover2_common::node>("lease_test_client");

        m_executor.add_node(m_bridge);
        m_executor.add_node(m_node);
        m_spin = std::thread([this] { m_executor.spin(); });

        m_fcu = std::make_shared<clover2_fcu::client>(
            clover2_fcu::context::node_interfaces(*m_node));

        m_activate_client =
            m_node->create_client<ActivateMode>("/fcu_bridge/activate_mode");
        m_mode_state_reader = m_node->create_subscription<ModeState>(
            "/fcu_bridge/mode_state", rclcpp::QoS(1).transient_local(),
            [this](ModeState::SharedPtr msg) {
                const std::scoped_lock lock(m_state_mutex);
                m_last_mode_state = std::move(msg);
            });
    }

    void TearDown() override {
        m_mode.reset();
        m_fcu.reset();
        m_executor.cancel();
        if (m_spin.joinable()) {
            m_spin.join();
        }
        m_bridge.reset();
        m_node.reset();
        m_plugin_env.uninstall();
    }

    void make_mode(const std::string& name) {
        clover2_fcu::mode_base::settings s;
        s.with_name(name);
        m_mode = std::make_shared<test_mode>(m_fcu, s);
    }

    void activate(const std::string& name) {
        auto request = std::make_shared<ActivateMode::Request>();
        request->name = name;
        const auto response = call_sync<ActivateMode>(
            m_activate_client, "/fcu_bridge/activate_mode", request,
            std::chrono::seconds(5));
        ASSERT_TRUE(response != nullptr);
        ASSERT_TRUE(response->ok) << response->error;
    }

    void publish_state(bool armed, clover2_fcu::data::flight_mode mode,
                       bool failsafe = false) {
        clover2_fcu::stub::system_state::publish({.connected = true,
                                                  .armed = armed,
                                                  .failsafe = failsafe,
                                                  .mode = mode});
    }

    std::shared_ptr<clover2_fcu::stub::commands> commands() {
        return std::static_pointer_cast<clover2_fcu::stub::commands>(
            m_fcu->get<clover2_fcu::capability::commands_interface>());
    }

    std::shared_ptr<clover2_fcu::stub::position_setpoint> position_setpoint() {
        return std::static_pointer_cast<clover2_fcu::stub::position_setpoint>(
            m_fcu->get<clover2_fcu::capability::setpoint_position_interface>());
    }

    template <typename Pred>
    bool wait_for(Pred pred, std::chrono::milliseconds timeout =
                                 std::chrono::milliseconds(5000)) const {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (pred()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return pred();
    }

    uint32_t last_mode_state_id() const {
        const std::scoped_lock lock(m_state_mutex);
        return m_last_mode_state ? m_last_mode_state->mode_id : 0;
    }

    stub_plugin_env m_plugin_env;
    std::shared_ptr<clover2_fcu::bridge> m_bridge;
    std::shared_ptr<clover2_common::node> m_node;
    clover2_fcu::client::shared_ptr m_fcu;
    std::shared_ptr<test_mode> m_mode;
    rclcpp::executors::SingleThreadedExecutor m_executor;
    std::thread m_spin;
    rclcpp::Client<ActivateMode>::SharedPtr m_activate_client;
    rclcpp::Subscription<ModeState>::SharedPtr m_mode_state_reader;
    mutable std::mutex m_state_mutex;
    ModeState::SharedPtr m_last_mode_state;
};

TEST_F(lease_test, duplicate_mode_names_are_rejected) {
    make_mode("dup");
    EXPECT_THROW(make_mode("dup"), clover2_fcu::exception);
}

TEST_F(lease_test, lease_granted_and_activated_from_inside) {
    make_mode("inflight");
    activate("inflight");
    EXPECT_TRUE(wait_for([&] { return m_mode->active(); }));

    publish_state(true, clover2_fcu::data::flight_mode::manual());
    EXPECT_TRUE(wait_for(
        [&] { return commands()->pending_count("set_mode:offboard") == 1; }));
    commands()->resolve_pending("set_mode:offboard", true);

    // OFFBOARD confirmed on the mode's own telemetry: the stream starts.
    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return m_mode->updates.load() > 0; }));
    EXPECT_FALSE(position_setpoint()->updates().empty());
    EXPECT_EQ(last_mode_state_id(), m_mode->mode_id());
}

TEST_F(lease_test, lease_revoked_on_mode_change) {
    make_mode("takeover");
    activate("takeover");
    EXPECT_TRUE(wait_for([&] { return m_mode->active(); }));

    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return m_mode->updates.load() > 0; }));

    publish_state(true, clover2_fcu::data::flight_mode::manual());
    EXPECT_TRUE(wait_for([&] { return !m_mode->active(); }));
    EXPECT_EQ(last_mode_state_id(), 0u);
}

TEST_F(lease_test, lease_revoked_on_disarm_and_failsafe) {
    make_mode("facts");
    activate("facts");
    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return m_mode->updates.load() > 0; }));

    publish_state(false, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return !m_mode->active(); }));

    // Re-arm and confirm again, then a failsafe fact.
    activate("facts");
    EXPECT_TRUE(wait_for([&] { return m_mode->active(); }));
    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return m_mode->updates.load() > 0; }));

    publish_state(true, clover2_fcu::data::flight_mode::offboard(),
                  /*failsafe=*/true);
    EXPECT_TRUE(wait_for([&] { return !m_mode->active(); }));
}

TEST_F(lease_test, stale_state_keeps_the_lease) {
    make_mode("stale");
    activate("stale");
    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return m_mode->updates.load() > 0; }));

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(m_mode->active());
    EXPECT_EQ(last_mode_state_id(), m_mode->mode_id());
}

TEST_F(lease_test, revoked_lease_restored_only_by_reactivation) {
    make_mode("restore");
    activate("restore");
    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return m_mode->active(); }));

    publish_state(true, clover2_fcu::data::flight_mode::manual());
    EXPECT_TRUE(wait_for([&] { return !m_mode->active(); }));

    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_mode->active());

    // Only a repeated ActivateMode does.
    activate("restore");
    EXPECT_TRUE(wait_for([&] { return m_mode->active(); }));
}

TEST_F(lease_test, empty_name_surrenders_the_lease) {
    make_mode("surrender");
    activate("surrender");
    publish_state(true, clover2_fcu::data::flight_mode::offboard());
    EXPECT_TRUE(wait_for([&] { return m_mode->active(); }));

    activate("");
    EXPECT_TRUE(wait_for([&] { return !m_mode->active(); }));
    EXPECT_EQ(last_mode_state_id(), 0u);
}

TEST_F(lease_test, client_provider_cache_race) {
    std::shared_ptr<clover2_fcu::capability::commands_interface> first;
    std::mutex first_mutex;
    std::atomic<int> mismatches{0};

    std::vector<std::thread> fetchers;
    for (int t = 0; t < 8; ++t) {
        fetchers.emplace_back([&] {
            auto provider =
                m_fcu->get<clover2_fcu::capability::commands_interface>();
            const std::scoped_lock lock(first_mutex);
            if (!first) {
                first = provider;
            } else if (first != provider) {
                ++mismatches;
            }
        });
    }
    for (auto& thread : fetchers) {
        thread.join();
    }

    EXPECT_EQ(mismatches.load(), 0);
    EXPECT_TRUE(first);
}

}  // namespace
