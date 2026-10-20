// clover2
#include <clover2_common/node.hpp>
#include <clover2_fcu/backend/backend_base.hpp>
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu_fake/factory.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// third party
#include <gtest/gtest.h>
#include <pluginlib/class_loader.hpp>

// STL
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace {

using clover2_fcu::command_handle;
using clover2_fcu::result;

class fake_env : public ::testing::Environment {
public:
    void SetUp() override { rclcpp::init(0, nullptr); }
    void TearDown() override { rclcpp::shutdown(); }
};

[[maybe_unused]] const auto* g_env =
    ::testing::AddGlobalTestEnvironment(new fake_env());

class commands_test : public ::testing::Test {
protected:
    void SetUp() override {
        clover2_fcu::fake::registry::instance().reset();

        m_loader =
            std::make_unique<pluginlib::ClassLoader<
                clover2_fcu::backend::backend_base>>(
                "clover2_fcu", "clover2_fcu::backend::backend_base");
        m_factory = m_loader->createSharedInstance("fake");

        m_node = std::make_shared<clover2_common::node>("fake_commands_test");
        m_base =
            std::make_shared<clover2_fcu::capability::base_interface_impl>(
                *m_node, "/fcu_bridge", nullptr);
        m_factory->initialize(m_base);
    }

    clover2_fcu::capability::commands_interface::shared_ptr commands() {
        return m_factory
            ->get<clover2_fcu::capability::commands_interface>();
    }

    clover2_fcu::fake::registry& registry() {
        return clover2_fcu::fake::registry::instance();
    }

    std::unique_ptr<pluginlib::ClassLoader<
        clover2_fcu::backend::backend_base>>
        m_loader;
    clover2_fcu::backend::backend_base::shared_ptr m_factory;
    std::shared_ptr<clover2_common::node> m_node;
    std::shared_ptr<clover2_fcu::capability::base_interface_impl> m_base;
};

TEST_F(commands_test, command_stays_pending_until_resolved) {
    auto h = commands()->arm();

    ASSERT_TRUE(h);
    EXPECT_FALSE(h->done());
    EXPECT_EQ(registry().pending_count("arm"), 1u);

    registry().resolve_pending("arm", result::value::success);
    EXPECT_EQ(h->wait_for(std::chrono::seconds(1)), result::value::success);
    EXPECT_EQ(registry().pending_count("arm"), 0u);
}

TEST_F(commands_test, on_done_fires_on_resolve) {
    auto h = commands()->land();

    int fires = 0;
    h->on_done([&](result r) {
        ++fires;
        EXPECT_EQ(r, result::value::rejected);
    });

    registry().resolve_pending("land", result::value::rejected);
    EXPECT_EQ(fires, 1);
}

TEST_F(commands_test, cancel_resolves_interrupted) {
    auto h = commands()->takeoff();

    h->cancel();

    EXPECT_TRUE(h->done());
    EXPECT_EQ(h->outcome(), result::value::interrupted);
}

TEST_F(commands_test, set_mode_is_logged_with_mode_name) {
    commands()->set_mode(clover2_fcu::data::flight_mode::value::offboard);

    const auto log = registry().command_log();
    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log.front(), "set_mode:offboard");
}

TEST_F(commands_test, two_pending_resolved_together) {
    auto first = commands()->rtl();
    auto second = commands()->rtl();

    EXPECT_EQ(registry().pending_count("rtl"), 2u);

    registry().resolve_pending("rtl", result::value::success);

    EXPECT_EQ(first->wait_for(std::chrono::seconds(1)),
              result::value::success);
    EXPECT_EQ(second->wait_for(std::chrono::seconds(1)),
              result::value::success);
}

}  // namespace
