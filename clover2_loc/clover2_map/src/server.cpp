// clover2
#include <clover2_map/server.hpp>

// ROS2
#include <rclcpp/logger.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_eigen/tf2_eigen.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

// STL
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <stdexcept>

namespace clover2_map {

server::server(const rclcpp::NodeOptions& options)
    : clover2_common::node("map_server", options) {
    auto diagnostic_interface = get_node_diagnostics_interface();
    diagnostic_interface->add<diagnostics::map_server_task>();

    declare_and_watch_parameter<std::string>(
        "map", "",
        [this](const rclcpp::Parameter& p) {  //
            auto new_file = std::filesystem::path(p.as_string());
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

            auto diagnostics = get_node_diagnostics_interface();
            diagnostics->get<diagnostics::map_server_task>().set_map_path(
                new_file.string());
            diagnostics->get<diagnostics::map_server_task>().set_provider(
                m_provider);
        },
        "Path to map file whit .txt/.yaml/.yml extension.");

    declare_and_watch_parameter<bool>(
        "send_tf", true,
        [this](const rclcpp::Parameter& p) { m_send_tf = p.as_bool(); },
        "Publish marker transforms as dynamic TF");
    m_send_tf = get_parameter("send_tf").as_bool();

    rclcpp::QoS qos_notification =
        rclcpp::QoS(rclcpp::KeepLast(1)).transient_local().reliable();
    m_map_update_pub = create_publisher<std_msgs::msg::Empty>("~/map_update",
                                                              qos_notification);
    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(this);

    auto map_group = get_node_base_interface()->get_default_callback_group();

    m_tf_timer = create_wall_timer(
        std::chrono::seconds(1), std::bind(&server::publish_tf_snapshot, this),
        map_group);

    m_map_service = create_service<clover2_pose_msgs::srv::GetMap>(
        "~/get_map",
        std::bind(&server::map_callback, this, std::placeholders::_1,
                  std::placeholders::_2),
        rclcpp::ServicesQoS(), map_group);

    m_modify_map_service = create_service<clover2_pose_msgs::srv::ModifyMap>(
        "~/modify_map",
        std::bind(&server::modify_map_callback, this, std::placeholders::_1,
                  std::placeholders::_2),
        rclcpp::ServicesQoS(), map_group);

    try {
        RCLCPP_INFO(get_logger(), "Using map '%s'",
                    m_provider->get_map().name.c_str());
        update_map();
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Start error: %s", e.what());
    }
}

void server::map_callback(
    const clover2_pose_msgs::srv::GetMap::Request::SharedPtr /* request */,
    clover2_pose_msgs::srv::GetMap::Response::SharedPtr response) {
    m_provider->get_map().to_msg(response->map);
}

void server::modify_map_callback(
    const clover2_pose_msgs::srv::ModifyMap::Request::SharedPtr request,
    clover2_pose_msgs::srv::ModifyMap::Response::SharedPtr response) {
    auto backup = m_provider->get_map();

    try {
        auto& m = m_provider->get_map();
        auto mk = clover2_map::marker::from_msg(request->marker);

        auto find_marker = [&m](int id) {
            return std::find_if(m.markers.begin(), m.markers.end(),
                                [id](const auto& x) { return x.id == id; });
        };

        using Request = clover2_pose_msgs::srv::ModifyMap::Request;

        switch (request->operation) {
            case Request::OPERATION_ADD: {
                if (find_marker(mk.id) != m.markers.end()) {
                    throw std::runtime_error("Marker " + std::to_string(mk.id) +
                                             " already exists");
                }

                if (mk.marker_frame_id.empty()) {
                    mk.marker_frame_id =
                        m.frame_id + "_aruco_" + std::to_string(mk.id);
                }

                m.markers.push_back(std::move(mk));
                break;
            }

            case Request::OPERATION_EDIT: {
                auto it = find_marker(mk.id);
                if (it == m.markers.end()) {
                    throw std::runtime_error("Marker " + std::to_string(mk.id) +
                                             " not found");
                }

                if (mk.marker_frame_id.empty()) {
                    mk.marker_frame_id =
                        m.frame_id + "_aruco_" + std::to_string(mk.id);
                }

                *it = std::move(mk);
                break;
            }

            case Request::OPERATION_DELETE: {
                auto it = find_marker(mk.id);
                if (it == m.markers.end()) {
                    throw std::runtime_error("Marker " + std::to_string(mk.id) +
                                             " not found");
                }

                m.markers.erase(it);
                break;
            }

            default:
                throw std::runtime_error("Unknown operation: " +
                                         std::to_string(request->operation));
        }

        m_provider->save();
    } catch (const std::exception& e) {
        m_provider->get_map() = std::move(backup);

        response->success = false;
        response->error_message = e.what();

        RCLCPP_ERROR(get_logger(), "Modify map failed: %s", e.what());
        return;
    }

    response->success = true;
    update_map();
}

void server::publish_tf_snapshot() {
    if (!m_send_tf || !m_provider) {
        return;
    }

    const auto& m = m_provider->get_map();

    for (const auto& it : m.markers) {
        if (!it.pose) {
            continue;
        }

        geometry_msgs::msg::TransformStamped transform;

        transform.header.frame_id = m.frame_id;
        transform.header.stamp = get_clock()->now();
        transform.child_frame_id = it.marker_frame_id;

        tf2::Transform t;
        tf2::fromMsg(tf2::toMsg(*it.pose), t);
        tf2::toMsg(t, transform.transform);

        m_tf_broadcaster->sendTransform(transform);
    }
}

void server::update_map() {
    publish_tf_snapshot();
    m_map_update_pub->publish(std_msgs::msg::Empty());
}

}  // namespace clover2_map

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_map::server)
