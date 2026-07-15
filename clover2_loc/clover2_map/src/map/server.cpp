#include <clover2/map/server.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <rclcpp/logger.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

namespace clover2::map {

server::server(const rclcpp::NodeOptions& options)
    : clover2_common::node("map_server", options) {
    declare_and_watch_parameter<std::string>(
        "map", "",
        [this](const rclcpp::Parameter& p) {  //
            auto new_file = std::filesystem::path(p.as_string());
            m_map_path = new_file.string();

            if (!std::filesystem::exists(new_file)) {
                throw std::runtime_error("File " + new_file.string() +
                                         " not exits");
            }

            if (!std::filesystem::is_regular_file(new_file)) {
                throw std::runtime_error(new_file.string() + " is not a file");
            }

            m_provider = std::make_shared<io::fs_provider>(
                new_file, get_logger().get_child("fs_provider"));

            m_provider->load();
            update_diagnostic_map_state();
        },
        "Path to map file whit .txt/.yaml/.yml extension.");

    rclcpp::QoS qos_notification =
        rclcpp::QoS(rclcpp::KeepLast(1)).transient_local().reliable();
    m_map_update_pub = create_publisher<std_msgs::msg::Empty>("~/map_update",
                                                              qos_notification);
    m_tf_static_broadcaster =
        std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);

    m_map_service = create_service<clover2_pose_msgs::srv::GetMap>(
        "~/get_map", std::bind(&server::map_callback, this,
                               std::placeholders::_1, std::placeholders::_2));

    m_diagnostics = std::make_shared<MapServerDiagnostics>(
        get_node_base_interface(), get_node_clock_interface(),
        get_node_logging_interface(), get_node_parameters_interface(),
        get_node_timers_interface(), get_node_topics_interface());
    m_diagnostics->set_diagnostic_callback(
        MapServerDiagnostics::diagnostic::map,
        std::bind(&server::produce_map_diagnostics, this,
                  std::placeholders::_1));
    m_diagnostics->set_diagnostic_callback(
        MapServerDiagnostics::diagnostic::interface,
        std::bind(&server::produce_interface_diagnostics, this,
                  std::placeholders::_1));

    try {
        RCLCPP_INFO(get_logger(), "Using map '%s'",
                    m_provider->get_map().name.c_str());
        update_diagnostic_map_state();
        update_map();
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Start error: %s", e.what());
    }
}

void server::map_callback(
    const clover2_pose_msgs::srv::GetMap::Request::SharedPtr /* request */,
    clover2_pose_msgs::srv::GetMap::Response::SharedPtr response) {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    ++m_get_map_requests;

    response->map = m_provider->get_map();
}

void server::update_map() {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    const auto& m = m_provider->get_map();

    std::vector<geometry_msgs::msg::TransformStamped> transforms;
    transforms.reserve(m.markers.size());

    for (const auto& it : m.markers) {
        geometry_msgs::msg::TransformStamped transform;

        transform.header.frame_id = m.header.frame_id;
        transform.header.stamp = get_clock()->now();

        transform.child_frame_id =
            m.header.frame_id + "_aruco_" + std::to_string(it.id);

        tf2::Transform t;
        tf2::fromMsg(it.pose.pose, t);
        tf2::toMsg(t, transform.transform);

        transforms.push_back(transform);
    }

    m_tf_static_broadcaster->sendTransform(transforms);
    m_map_update_pub->publish(std_msgs::msg::Empty());

    m_static_tf_count = transforms.size();
    ++m_map_updates;
}

void server::update_diagnostic_map_state() {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    if (!m_provider) {
        m_map_loaded = false;
        m_map_frame_id.clear();
        m_marker_count = 0;
        return;
    }

    const auto& map = m_provider->get_map();
    m_map_loaded = true;
    m_map_frame_id = map.header.frame_id;
    m_marker_count = map.markers.size();
}

void server::produce_map_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    if (!m_map_loaded) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Map not loaded");
    } else if (m_marker_count == 0) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Map loaded but empty");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "Map loaded");
    }

    stat.add("Map frame", m_map_frame_id.empty() ? "unknown" : m_map_frame_id);
    stat.add("Map path", m_map_path.empty() ? "unknown" : m_map_path);
    stat.add("Marker count", std::to_string(m_marker_count));
}

void server::produce_interface_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    const bool service_available = static_cast<bool>(m_map_service);
    const bool map_update_available = static_cast<bool>(m_map_update_pub);

    if (!service_available || !map_update_available) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Map server interface is not available");
    } else if (m_map_loaded && m_static_tf_count != m_marker_count) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Static TF transform count mismatch");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Interfaces available");
    }

    stat.add("Get map requests", std::to_string(m_get_map_requests));
    stat.add("Map updates", std::to_string(m_map_updates));
    stat.add("Static TF transforms", std::to_string(m_static_tf_count));
    stat.add("Map update subscribers",
             m_map_update_pub ? m_map_update_pub->get_subscription_count() : 0);
}

}  // namespace clover2::map

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::map::server)
