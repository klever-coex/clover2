// clover2
#include <clover2_fcu_bridge/backend/context.hpp>
#include <clover2_fcu_bridge/backend/fabric.hpp>
#include <clover2_fcu_bridge/offboard.hpp>
#include <clover2_fcu_bridge/server.hpp>

// ROS2
#include <rclcpp/node_options.hpp>
#include <rclcpp/qos.hpp>
#include <tf2/utils.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <algorithm>
#include <chrono>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace {

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
    m_parameter_watcher =
        std::make_shared<clover2_common::parameter_watcher>(*this);

    enable_diagnostic_updater();

    m_service_callback_group =
        create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    m_parameter_watcher->declare_and_watch_parameter<std::string>(
        "backend", m_backend_name,
        [this](const rclcpp::Parameter& p) {
            const std::string name = p.as_string();
            ensure_backend_registered(name);
            m_backend_name = name;
        },
        "Offboard backend name for fabric (e.g. mavros)");

    m_parameter_watcher->declare_and_watch_parameter<double>(
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

    m_parameter_watcher->declare_and_watch_parameter<double>(
        "tolerance", m_tolerance,
        [this](const rclcpp::Parameter& p) {
            m_tolerance = p.as_double();

            if (m_offboard.get()) {
                m_offboard->set_tolerance(m_tolerance);
            }
        },
        "Controller tolerance");

    m_parameter_watcher->declare_and_watch_parameter<double>(
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
        m_offboard->set_slowdown_distance(m_slowdown)
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
    m_offboard.reset();
    m_backend.reset();

    RCLCPP_INFO(get_logger(), "cleanup");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    RCLCPP_INFO(get_logger(), "shutdown");
    return CallbackReturn::SUCCESS;
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
    [[maybe_unused]] const rclcpp_action::GoalUUID& uuid,
    std::shared_ptr<const NavigateAsync::Goal> goal) {
    try {
        const double speed =
            std::isnan(goal->speed) ? 0.0 : static_cast<double>(goal->speed);
        std::optional<double> x, y, z, yaw;
        extract_target_pose(goal->pose, x, y, z, yaw);

        m_offboard->navigate(goal->header.frame_id, x, y, z, yaw, speed);
    } catch (const std::exception& e) {
        RCLCPP_WARN(get_logger(), "Unable to navigate: %s", e.what());
        m_offboard->reset_state();
        return rclcpp_action::GoalResponse::REJECT;
    }

    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse server::handle_navigate_async_cancel(
    [[maybe_unused]] const std::shared_ptr<GoalHandleNavigateAsync>
        goal_handle) {
    m_offboard->reset_state();
    return rclcpp_action::CancelResponse::ACCEPT;
}

void server::handle_navigate_async_accepted(
    [[maybe_unused]] const std::shared_ptr<GoalHandleNavigateAsync>
        goal_handle) {
    m_offboard->set_process_callback(
        std::bind(&server::process_navigate_async, this, goal_handle));
}

void server::process_navigate_async(
    const std::shared_ptr<GoalHandleNavigateAsync> goal_handle) {
    if (m_offboard->in_error()) {
        auto result = std::make_shared<NavigateAsync::Result>();
        result->success = false;
        result->message = "offboard error";
        goal_handle->abort(result);
        m_offboard->set_process_callback(nullptr);
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

    if (m_offboard->in_idle()) {
        RCLCPP_INFO(get_logger(), "Navigate async finished");
        auto result = std::make_shared<NavigateAsync::Result>();
        goal_handle->succeed(result);
        m_offboard->set_process_callback(nullptr);
    }
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
