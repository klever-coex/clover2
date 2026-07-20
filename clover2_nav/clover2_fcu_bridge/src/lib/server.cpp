// clover2
#include <clover2_fcu_bridge/backend/context.hpp>
#include <clover2_fcu_bridge/backend/fabric.hpp>
#include <clover2_fcu_bridge/offboard.hpp>
#include <clover2_fcu_bridge/server.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/qos.hpp>
#include <tf2/utils.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

constexpr double kNavigateBondConnectTimeout = 2.0;
constexpr double kNavigateBondHeartbeatPeriod = 0.2;
constexpr double kNavigateBondHeartbeatTimeout = 1.0;
constexpr double kSensorStaleTimeoutSec = 1.0;
constexpr float kBatteryWarnPercentage = 0.20F;
constexpr float kBatteryErrorPercentage = 0.10F;
constexpr auto kNavigateCompletionGracePeriod = std::chrono::milliseconds(1200);

void ensure_backend_registered(const std::string& name) {
    const auto backends = clover2_fcu_bridge::offboard::list_backends();
    if (std::find(backends.begin(), backends.end(), name) == backends.end()) {
        std::string avail;
        for (size_t i = 0; i < backends.size(); ++i) {
            if (i > 0) {
                avail += ", ";
            }
            avail += backends[i];
        }
        throw std::invalid_argument("unknown backend '" + name +
                                    "'; available: " + avail);
    }
}

}  // namespace

namespace clover2_fcu_bridge {

server::server(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("fcu_bridge", options)
    , m_backend_name("mavros") {
    auto diagnostics = std::make_shared<ServerDiagnostics>(
        get_node_base_interface(), get_node_clock_interface(),
        get_node_logging_interface(), get_node_parameters_interface(),
        get_node_timers_interface(), get_node_topics_interface());
    diagnostics->set_diagnostic_callback(
        ServerDiagnostics::diagnostic::fcu_state,
        std::bind(&server::produce_fcu_state_diagnostics, this,
                  std::placeholders::_1));
    diagnostics->set_diagnostic_callback(
        ServerDiagnostics::diagnostic::power,
        std::bind(&server::produce_power_diagnostics, this,
                  std::placeholders::_1));
    diagnostics->set_diagnostic_callback(
        ServerDiagnostics::diagnostic::imu,
        std::bind(&server::produce_imu_diagnostics, this,
                  std::placeholders::_1));
    diagnostics->set_diagnostic_callback(
        ServerDiagnostics::diagnostic::barometer,
        std::bind(&server::produce_barometer_diagnostics, this,
                  std::placeholders::_1));
    diagnostics->set_diagnostic_callback(
        ServerDiagnostics::diagnostic::interfaces,
        std::bind(&server::produce_interfaces_diagnostics, this,
                  std::placeholders::_1));
    diagnostics->set_diagnostic_callback(
        ServerDiagnostics::diagnostic::navigation,
        std::bind(&server::produce_navigation_diagnostics, this,
                  std::placeholders::_1));
    set_node_diagnostics_interface(std::move(diagnostics));

    m_service_callback_group =
        create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    declare_and_watch_parameter<std::string>(
        "backend", m_backend_name,
        [this](const rclcpp::Parameter& p) {
            const std::string name = p.as_string();
            ensure_backend_registered(name);
            m_backend_name = name;
        },
        "Offboard backend name for fabric (e.g. mavros)");

    declare_and_watch_parameter<double>(
        "speed_limit", m_speed_limit,
        [this](const rclcpp::Parameter& p) {
            if (p.as_double() < 0.1) {
                throw std::runtime_error(
                    "Speed limit should be greater then 0.1");
            }

            m_speed_limit = p.as_double();
            if (m_offboard.get()) {
                m_offboard->set_speed_limit(m_speed_limit);
            }
        },
        "Controller speed limit");

    declare_and_watch_parameter<double>(
        "tolerance", m_tolerance,
        [this](const rclcpp::Parameter& p) {
            m_tolerance = p.as_double();

            if (m_offboard.get()) {
                m_offboard->set_tolerance(m_tolerance);
            }
        },
        "Controller tolerance");

    declare_and_watch_parameter<double>(
        "slowdown_distance", m_slowdown,
        [this](const rclcpp::Parameter& p) {
            m_slowdown = p.as_double();

            if (m_offboard.get()) {
                m_offboard->set_slowdown_distance(m_slowdown);
            }
        },
        "Controller slowdown distance");

    register_on_configure(
        std::bind(&server::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&server::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&server::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&server::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&server::on_shutdown, this, std::placeholders::_1));
}

server::CallbackReturn server::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    try {
        clover2_fcu_bridge::backend::context ctx(*this);
        m_backend = backend::fabric::instance().create(m_backend_name, ctx);
        m_offboard = offboard::make_shared(this->shared_from_this(), m_backend);
        m_offboard->set_speed_limit(m_speed_limit);
        m_offboard->set_tolerance(m_tolerance);
        m_offboard->set_slowdown_distance(m_slowdown);
    } catch (const std::runtime_error& e) {
        RCLCPP_ERROR(get_logger(), "Failed to create backend '%s': %s",
                     m_backend_name.c_str(), e.what());
        return CallbackReturn::FAILURE;
    }

    RCLCPP_INFO(get_logger(), "configure");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    using namespace std::placeholders;

    m_state_pub = create_publisher<clover2_nav_msgs::msg::State>(
        "~/state", rclcpp::QoS(10));

    m_arm_disarm_srv = create_service<clover2_nav_msgs::srv::ArmDisarm>(
        "~/arm_disarm", std::bind(&server::handle_arm_disarm, this, _1, _2),
        rclcpp::ServicesQoS(), m_service_callback_group);

    m_land_srv = create_service<clover2_nav_msgs::srv::Land>(
        "~/land", std::bind(&server::handle_land, this, _1, _2),
        rclcpp::ServicesQoS(), m_service_callback_group);

    m_set_position_srv = create_service<clover2_nav_msgs::srv::SetPosition>(
        "~/set_position", std::bind(&server::handle_set_position, this, _1, _2),
        rclcpp::ServicesQoS(), m_service_callback_group);

    m_navigate_srv = create_service<clover2_nav_msgs::srv::Navigate>(
        "~/navigate", std::bind(&server::handle_navigate, this, _1, _2),
        rclcpp::ServicesQoS(), m_service_callback_group);

    m_navigate_async_action = rclcpp_action::create_server<NavigateAsync>(
        this, "~/navigate_async",
        std::bind(&server::handle_navigate_async_goal, this, _1, _2),
        std::bind(&server::handle_navigate_async_cancel, this, _1),
        std::bind(&server::handle_navigate_async_accepted, this, _1));

    m_state_publish_timer =
        create_wall_timer(std::chrono::milliseconds(100), [this]() {
            clover2_nav_msgs::msg::State state;
            state.is_armed = m_backend->is_armed();
            state.mode = m_backend->get_mode().to_str();
            m_state_pub->publish(state);
        });

    RCLCPP_INFO(get_logger(), "activate");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    cleanup_navigate_bond();
    m_navigate_goal_pending.store(false);
    m_active_navigate_goal.reset();
    m_navigate_completion_started_at.reset();

    m_state_pub.reset();

    m_arm_disarm_srv.reset();
    m_land_srv.reset();
    m_navigate_srv.reset();
    m_set_position_srv.reset();

    m_state_publish_timer.reset();

    RCLCPP_INFO(get_logger(), "deactivate");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    cleanup_navigate_bond();
    m_navigate_goal_pending.store(false);
    m_active_navigate_goal.reset();
    m_navigate_completion_started_at.reset();

    m_offboard.reset();
    m_backend.reset();

    RCLCPP_INFO(get_logger(), "cleanup");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    cleanup_navigate_bond();
    m_navigate_goal_pending.store(false);
    m_active_navigate_goal.reset();
    m_navigate_completion_started_at.reset();

    RCLCPP_INFO(get_logger(), "shutdown");
    return CallbackReturn::SUCCESS;
}

void server::produce_fcu_state_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_backend) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Backend is not initialized");
        return;
    }

    const auto state = m_backend->get_fcu_state_snapshot();

    if (!state.received) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "FCU state is not received");
    } else if (!state.connected) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "FCU is not connected");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "FCU connected");
    }

    stat.add("Connected", state.connected ? "true" : "false");
    stat.add("State received", state.received ? "true" : "false");
    stat.add("Armed", state.armed ? "true" : "false");
    stat.add("Mode", state.mode);
}

void server::produce_power_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_backend) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Backend is not initialized");
        return;
    }

    const auto power = m_backend->get_power_snapshot();

    if (!power.received) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Battery status is not received");
    } else if (!std::isfinite(power.percentage)) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Battery percentage is not available");
    } else if (power.percentage <= kBatteryErrorPercentage) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Battery charge is critically low");
    } else if (power.percentage <= kBatteryWarnPercentage) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Battery charge is low");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Battery charge is normal");
    }

    stat.add("Battery received", power.received ? "true" : "false");
    stat.add("Voltage", power.received ? power.voltage : NAN);
    stat.add("Percentage", power.received ? power.percentage : NAN);
}

void server::produce_imu_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_backend) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Backend is not initialized");
        return;
    }

    const auto imu = m_backend->get_imu_snapshot();
    const double age = imu.received ? (now() - imu.stamp).seconds() : NAN;

    if (!imu.received) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "IMU is not received");
    } else if (age > kSensorStaleTimeoutSec) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "IMU data is stale");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "IMU data is fresh");
    }

    stat.add("IMU received", imu.received ? "true" : "false");
    stat.add("IMU age, sec", imu.received ? age : NAN);
}

void server::produce_barometer_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_backend) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Backend is not initialized");
        return;
    }

    const auto barometer = m_backend->get_barometer_snapshot();
    const double age =
        barometer.received ? (now() - barometer.stamp).seconds() : NAN;

    if (!barometer.received) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Barometer is not received");
    } else if (age > kSensorStaleTimeoutSec) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Barometer data is stale");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Barometer data is fresh");
    }

    stat.add("Barometer received", barometer.received ? "true" : "false");
    stat.add("Barometer age, sec", barometer.received ? age : NAN);
    stat.add("Pressure, Pa",
             barometer.received ? barometer.msg.fluid_pressure : NAN);
}

void server::produce_interfaces_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    const bool state_publisher = static_cast<bool>(m_state_pub);
    const bool arm_disarm_service = static_cast<bool>(m_arm_disarm_srv);
    const bool land_service = static_cast<bool>(m_land_srv);
    const bool set_position_service = static_cast<bool>(m_set_position_srv);
    const bool navigate_service = static_cast<bool>(m_navigate_srv);
    const bool navigate_async_action = static_cast<bool>(m_navigate_async_action);
    const bool state_timer = static_cast<bool>(m_state_publish_timer);

    if (state_publisher && arm_disarm_service && land_service &&
        set_position_service && navigate_service && navigate_async_action &&
        state_timer) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Interfaces available");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Interfaces are not active");
    }

    stat.add("State publisher", state_publisher ? "true" : "false");
    stat.add("Arm/disarm service", arm_disarm_service ? "true" : "false");
    stat.add("Land service", land_service ? "true" : "false");
    stat.add("Set position service", set_position_service ? "true" : "false");
    stat.add("Navigate service", navigate_service ? "true" : "false");
    stat.add("Navigate async action",
             navigate_async_action ? "true" : "false");
    stat.add("State timer", state_timer ? "true" : "false");
}

void server::produce_navigation_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    const bool goal_pending = m_navigate_goal_pending.load();
    const bool active_goal = static_cast<bool>(m_active_navigate_goal);
    const bool bond_active = static_cast<bool>(m_navigate_bond);

    if (active_goal && !bond_active) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Active navigation goal has no bond");
    } else if (goal_pending) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Navigation goal is pending");
    } else if (active_goal) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Navigation goal is active");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Navigation idle");
    }

    stat.add("Goal pending", goal_pending ? "true" : "false");
    stat.add("Active goal", active_goal ? "true" : "false");
    stat.add("Navigate bond active", bond_active ? "true" : "false");
    stat.add("Navigate bond id",
             m_navigate_bond_id.empty() ? "none" : m_navigate_bond_id);
}

void server::start_navigate_bond(
    const std::shared_ptr<GoalHandleNavigateAsync> goal_handle) {
    if (m_navigate_bond) {
        RCLCPP_WARN(get_logger(), "Replacing an existing navigate action bond");
        cleanup_navigate_bond();
    }

    const std::string bond_id =
        "navigate_async:" +
        rclcpp_action::to_string(goal_handle->get_goal_id());
    m_navigate_bond_id = bond_id;
    m_navigate_bond_closing = false;
    m_navigate_bond = std::make_shared<bond::Bond>(
        "/fcu_bridge/bond", bond_id, get_node_base_interface(),
        get_node_logging_interface(), get_node_parameters_interface(),
        get_node_timers_interface(), get_node_topics_interface());
    m_navigate_bond->setConnectTimeout(kNavigateBondConnectTimeout);
    m_navigate_bond->setHeartbeatPeriod(kNavigateBondHeartbeatPeriod);
    m_navigate_bond->setHeartbeatTimeout(kNavigateBondHeartbeatTimeout);

    // The UUID and goal handle allow you to filter the callback from the old
    // action.
    m_navigate_bond->setFormedCallback([this, bond_id, goal_handle]() {
        handle_navigate_bond_formed(bond_id, goal_handle);
    });
    m_navigate_bond->setBrokenCallback(
        [this, bond_id]() { handle_navigate_bond_broken(bond_id); });
    m_navigate_bond->start();

    RCLCPP_INFO(get_logger(), "Started navigate action bond: %s",
                bond_id.c_str());
}

void server::cleanup_navigate_bond() {
    m_navigate_completion_started_at.reset();

    if (!m_navigate_bond) {
        m_navigate_bond_id.clear();
        return;
    }

    const std::string bond_id = m_navigate_bond_id;
    auto bond = std::move(m_navigate_bond);

    // This flag separates an expected breakBond() from a lost client.
    m_navigate_bond_closing = true;
    m_navigate_bond_id.clear();
    bond->breakBond();
    m_navigate_bond_closing = false;

    RCLCPP_INFO(get_logger(), "Stopped navigate action bond: %s",
                bond_id.c_str());
}

void server::handle_navigate_bond_formed(
    const std::string& bond_id,
    const std::shared_ptr<GoalHandleNavigateAsync> goal_handle) {
    if (bond_id != m_navigate_bond_id || m_navigate_bond_closing ||
        goal_handle != m_active_navigate_goal) {
        return;
    }

    RCLCPP_INFO(get_logger(), "Navigate action bond formed: %s",
                bond_id.c_str());

    if (goal_handle->is_canceling()) {
        auto result = std::make_shared<NavigateAsync::Result>();
        result->success = false;
        result->message = "navigate canceled";
        goal_handle->canceled(result);

        cleanup_navigate_bond();
        m_active_navigate_goal.reset();
        return;
    }

    // The drone is not controlled before the bond is formed.
    try {
        const auto goal = goal_handle->get_goal();
        const double speed =
            std::isnan(goal->speed) ? 0.0 : static_cast<double>(goal->speed);
        std::optional<double> x, y, z, yaw;
        extract_target_pose(goal->pose, x, y, z, yaw);

        m_offboard->navigate(goal->header.frame_id, x, y, z, yaw, speed);
        m_offboard->set_process_callback(
            std::bind(&server::process_navigate_async, this, goal_handle));
    } catch (const std::exception& e) {
        RCLCPP_WARN(get_logger(), "Unable to navigate: %s", e.what());
        m_offboard->reset_state();

        if (goal_handle->is_active()) {
            auto result = std::make_shared<NavigateAsync::Result>();
            result->success = false;
            result->message = e.what();
            goal_handle->abort(result);
        }

        cleanup_navigate_bond();
        m_active_navigate_goal.reset();
    }
}

void server::handle_navigate_bond_broken(const std::string& bond_id) {
    if (bond_id != m_navigate_bond_id || m_navigate_bond_closing) {
        return;
    }

    RCLCPP_WARN(get_logger(), "Navigate action bond broken: %s",
                bond_id.c_str());
    auto goal_handle = m_active_navigate_goal;

    // Clear the bond first so a repeated callback cannot handle the same break.
    m_navigate_bond.reset();
    m_navigate_bond_id.clear();
    m_navigate_goal_pending.store(false);
    m_navigate_completion_started_at.reset();

    // Stop offboard control first, then request landing.
    if (m_offboard) {
        try {
            m_offboard->set_process_callback(nullptr);
            m_offboard->reset_state();
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(),
                         "Reset after navigate action bond break failed: %s",
                         e.what());
        }
    }

    if (m_backend) {
        try {
            m_backend->land();
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(),
                         "Landing after navigate action bond break failed: %s",
                         e.what());
        }
    } else {
        RCLCPP_WARN(get_logger(),
                    "Unable to land after navigate action bond break: "
                    "backend is not initialized");
    }

    if (goal_handle && goal_handle->is_active()) {
        try {
            auto result = std::make_shared<NavigateAsync::Result>();
            result->success = false;
            result->message = "navigate action bond broken";
            goal_handle->abort(result);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(),
                         "Abort after navigate action bond break failed: %s",
                         e.what());
        }
    }

    m_active_navigate_goal.reset();
}

void server::handle_arm_disarm(
    const clover2_nav_msgs::srv::ArmDisarm::Request::SharedPtr req,
    clover2_nav_msgs::srv::ArmDisarm::Response::SharedPtr resp) {
    try {
        if (req->arm) {
            m_backend->arm();
        } else {
            m_backend->disarm();
        }
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "arm/disarm failed: %s", e.what());
        resp->success = false;
        resp->message = e.what();
        return;
    }
}

void server::handle_land(
    [[maybe_unused]] const clover2_nav_msgs::srv::Land::Request::SharedPtr req,
    clover2_nav_msgs::srv::Land::Response::SharedPtr resp) {
    try {
        m_offboard->reset_state();
        m_backend->land();
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "arm/disarm failed: %s", e.what());
        resp->success = false;
        resp->message = e.what();
        return;
    }
}

void server::handle_set_position(
    const clover2_nav_msgs::srv::SetPosition::Request::SharedPtr req,
    clover2_nav_msgs::srv::SetPosition::Response::SharedPtr resp) {
    std::optional<double> x, y, z, yaw;
    extract_target_pose(req->pose, x, y, z, yaw);

    try {
        m_offboard->set_position(req->header.frame_id, x, y, z, yaw);
    } catch (const std::runtime_error& e) {
        RCLCPP_WARN(get_logger(), "set position failed: %s", e.what());
        resp->success = false;
        resp->message = e.what();
        return;
    }
}

void server::handle_navigate(
    const clover2_nav_msgs::srv::Navigate::Request::SharedPtr req,
    clover2_nav_msgs::srv::Navigate::Response::SharedPtr resp) {
    std::optional<double> x, y, z, yaw;
    extract_target_pose(req->pose, x, y, z, yaw);

    const double speed =
        std::isnan(req->speed) ? 0.0 : static_cast<double>(req->speed);

    try {
        m_offboard->navigate(req->header.frame_id, x, y, z, yaw, speed);
    } catch (const std::runtime_error& e) {
        RCLCPP_WARN(get_logger(), "navigation failed: %s", e.what());
        resp->success = false;
        resp->message = e.what();
        return;
    }
}

rclcpp_action::GoalResponse server::handle_navigate_async_goal(
    const rclcpp_action::GoalUUID& uuid,
    [[maybe_unused]] std::shared_ptr<const NavigateAsync::Goal> goal) {
    const auto goal_uuid = rclcpp_action::to_string(uuid);
    RCLCPP_INFO(get_logger(), "Navigate async goal received: %s",
                goal_uuid.c_str());

    const bool goal_was_pending = m_navigate_goal_pending.exchange(true);
    if (goal_was_pending || m_active_navigate_goal) {
        if (!goal_was_pending) {
            m_navigate_goal_pending.store(false);
        }
        RCLCPP_WARN(get_logger(), "Navigate async goal rejected: goal active");
        return rclcpp_action::GoalResponse::REJECT;
    }

    // Reserve the slot until the accepted callback without starting navigation
    // early.
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse server::handle_navigate_async_cancel(
    const std::shared_ptr<GoalHandleNavigateAsync> goal_handle) {
    RCLCPP_INFO(get_logger(), "Navigate async cancel requested");

    if (goal_handle != m_active_navigate_goal) {
        RCLCPP_WARN(get_logger(),
                    "Navigate async cancel rejected: goal inactive");
        return rclcpp_action::CancelResponse::REJECT;
    }

    m_offboard->reset_state();
    return rclcpp_action::CancelResponse::ACCEPT;
}

void server::handle_navigate_async_accepted(
    const std::shared_ptr<GoalHandleNavigateAsync> goal_handle) {
    RCLCPP_INFO(get_logger(), "Navigate async goal accepted");
    m_active_navigate_goal = goal_handle;
    m_navigate_goal_pending.store(false);
    m_navigate_completion_started_at.reset();
    start_navigate_bond(goal_handle);
}

void server::process_navigate_async(
    const std::shared_ptr<GoalHandleNavigateAsync> goal_handle) {
    if (goal_handle != m_active_navigate_goal) {
        return;
    }

    if (goal_handle->is_canceling()) {
        RCLCPP_INFO(get_logger(), "Navigate async canceled");
        auto result = std::make_shared<NavigateAsync::Result>();
        result->success = false;
        result->message = "navigate canceled";
        goal_handle->canceled(result);

        // On terminal state, remove the callback and the bond owned by this goal.
        m_offboard->set_process_callback(nullptr);
        cleanup_navigate_bond();
        m_active_navigate_goal.reset();
        return;
    }

    if (m_offboard->in_error()) {
        auto result = std::make_shared<NavigateAsync::Result>();
        result->success = false;
        result->message = "offboard error";
        RCLCPP_WARN(get_logger(), "Navigate async aborted: offboard error");
        goal_handle->abort(result);

        m_offboard->set_process_callback(nullptr);
        cleanup_navigate_bond();
        m_active_navigate_goal.reset();
        return;
    }

    auto feedback = std::make_shared<NavigateAsync::Feedback>();
    double yaw;
    tf2::Vector3 diff;
    m_offboard->nav_current_diff(diff, yaw);
    feedback->distance = tf2::toMsg(diff);
    feedback->yaw_distance = yaw;

    RCLCPP_DEBUG(get_logger(),
                 "Navigate async feedback, distance: %.3f, yaw_distance: %.3f",
                 diff.length(), feedback->yaw_distance);

    goal_handle->publish_feedback(feedback);

    if (!m_offboard->in_idle()) {
        m_navigate_completion_started_at.reset();
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (!m_navigate_completion_started_at) {
        m_navigate_completion_started_at = now;
        RCLCPP_INFO(
            get_logger(),
            "Navigate async target reached, waiting for bond grace period");
        return;
    }

    if (now - *m_navigate_completion_started_at <
        kNavigateCompletionGracePeriod) {
        return;
    }

    RCLCPP_INFO(get_logger(), "Navigate async finished");
    auto result = std::make_shared<NavigateAsync::Result>();
    goal_handle->succeed(result);

    m_offboard->set_process_callback(nullptr);
    cleanup_navigate_bond();
    m_active_navigate_goal.reset();
}

void server::extract_target_pose(const geometry_msgs::msg::Pose& pose,
                                 std::optional<double>& x,
                                 std::optional<double>& y,
                                 std::optional<double>& z,
                                 std::optional<double>& yaw) const {
    const auto& p = pose.position;

    x = std::nullopt;
    y = std::nullopt;
    z = std::nullopt;
    yaw = std::nullopt;

    if (!std::isnan(p.x)) {
        x = p.x;
    }

    if (!std::isnan(p.y)) {
        y = p.y;
    }

    if (!std::isnan(p.z)) {
        z = p.z;
    }

    if (!std::isnan(pose.orientation.w)) {
        yaw = tf2::getYaw(pose.orientation);
    }

    RCLCPP_INFO(get_logger(),
                "extract_target_pose x=%.3f y=%.3f z=%.3f yaw=%.3f", p.x, p.y,
                p.z, yaw.value_or(NAN));
}

}  // namespace clover2_fcu_bridge

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_fcu_bridge::server)
