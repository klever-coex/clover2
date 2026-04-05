#include "clover2_offboard_msgs/srv/navigate.hpp"
#include <clover2_offboard/backend/context.hpp>
#include <clover2_offboard/backend/fabric.hpp>
#include <clover2_offboard/helper.hpp>
#include <clover2_offboard/server.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/qos.hpp>
#include <tf2/utils.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace clover2_offboard {

namespace {

void ensure_backend_registered(const std::string& name) {
    const auto backends = helper::list_backends();
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

server::server(const rclcpp::NodeOptions& options)
    : clover2::common::lifecycle_node("offboard_server", options)
    , m_backend_name("mavros") {
    m_parameter_watcher =
        std::make_shared<clover2::common::parameter_watcher>(*this);

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
    RCLCPP_INFO(get_logger(), "configure");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    try {
        m_helper = clover2_offboard::helper::make_shared(
            this->shared_from_this(), m_backend_name);
    } catch (const std::runtime_error& e) {
        RCLCPP_ERROR(get_logger(), "Failed to create backend '%s': %s",
                     m_backend_name.c_str(), e.what());
        return CallbackReturn::FAILURE;
    }

    m_set_position_srv =
        create_service<clover2_offboard_msgs::srv::SetPosition>(
            "~/set_position",
            std::bind(&server::handle_set_position, this, std::placeholders::_1,
                      std::placeholders::_2),
            rclcpp::ServicesQoS(), m_service_callback_group);

    m_navigate_srv = create_service<clover2_offboard_msgs::srv::Navigate>(
        "~/navigate",
        std::bind(&server::handle_navigation, this, std::placeholders::_1,
                  std::placeholders::_2),
        rclcpp::ServicesQoS(), m_service_callback_group);

    RCLCPP_INFO(get_logger(), "activate");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_navigate_srv.reset();
    m_set_position_srv.reset();
    m_helper.reset();
    RCLCPP_INFO(get_logger(), "deactivate");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    RCLCPP_INFO(get_logger(), "cleanup");
    return CallbackReturn::SUCCESS;
}

server::CallbackReturn server::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    RCLCPP_INFO(get_logger(), "shutdown");
    return CallbackReturn::SUCCESS;
}

void server::handle_set_position(
    const std::shared_ptr<clover2_offboard_msgs::srv::SetPosition::Request>
        request,
    std::shared_ptr<clover2_offboard_msgs::srv::SetPosition::Response>
        response) {
    const auto& p = request->pose.position;
    const float value_yaw =
        static_cast<float>(tf2::getYaw(request->pose.orientation));
    RCLCPP_DEBUG(get_logger(),
                 "set_position frame=%s x=%.3f y=%.3f z=%.3f yaw=%.3f",
                 request->header.frame_id.c_str(), p.x, p.y, p.z, value_yaw);

    std::optional<double> x = std::nullopt;
    std::optional<double> y = std::nullopt;
    std::optional<double> z = std::nullopt;
    std::optional<double> yaw = std::nullopt;

    if (!std::isnan(p.x)) {
        x = p.x;
    }

    if (!std::isnan(p.y)) {
        y = p.y;
    }

    if (!std::isnan(p.z)) {
        z = p.z;
    }

    if (!std::isnan(value_yaw)) {
        yaw = value_yaw;
    }

    try {
        m_helper->set_position(request->header.frame_id, x, y, z, yaw);
    } catch (const std::runtime_error& e) {
        RCLCPP_WARN(get_logger(), "set position failed: %s", e.what());
        response->success = false;
        response->message = e.what();
        return;
    }

    response->success = true;
    response->message = "ok";
}

void server::handle_navigation(
    const std::shared_ptr<clover2_offboard_msgs::srv::Navigate::Request>
        request,
    std::shared_ptr<clover2_offboard_msgs::srv::Navigate::Response> response) {
    const auto& p = request->pose.position;
    const float value_yaw =
        static_cast<float>(tf2::getYaw(request->pose.orientation));
    RCLCPP_DEBUG(get_logger(),
                 "navigation frame=%s speed=%.3f x=%.3f y=%.3f z=%.3f yaw=%.3f",
                 request->header.frame_id.c_str(), request->speed, p.x, p.y,
                 p.z, value_yaw);

    std::optional<double> x = std::nullopt;
    std::optional<double> y = std::nullopt;
    std::optional<double> z = std::nullopt;
    std::optional<double> yaw = std::nullopt;

    if (!std::isnan(p.x)) {
        x = p.x;
    }

    if (!std::isnan(p.y)) {
        y = p.y;
    }

    if (!std::isnan(p.z)) {
        z = p.z;
    }

    if (!std::isnan(value_yaw)) {
        yaw = value_yaw;
    }

    const double speed =
        std::isnan(request->speed) ? 0.3 : static_cast<double>(request->speed);

    try {
        m_helper->navigate(request->header.frame_id, x, y, z, yaw, speed);
    } catch (const std::runtime_error& e) {
        RCLCPP_WARN(get_logger(), "navigation failed: %s", e.what());
        response->success = false;
        response->message = e.what();
        return;
    }

    response->success = true;
    response->message = "ok";
}

}  // namespace clover2_offboard
