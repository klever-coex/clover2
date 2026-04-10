#include <clover2/map/server.hpp>
#include <rclcpp/logger.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <filesystem>
#include <memory>
#include <stdexcept>

namespace clover2::map {

server::server(const rclcpp::NodeOptions& options)
    : rclcpp::Node("map_server", options)
    , m_parameter_watcher(*this) {
    m_parameter_watcher.declare_and_watch_parameter<std::string>(
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
        },
        "Path to map file whit .txt/.yaml/.yml extension.");

    m_map_update_pub = create_publisher<std_msgs::msg::Empty>(
        "~/map_update", rclcpp::SensorDataQoS());
    m_tf_static_broadcaster =
        std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);

    m_map_service = create_service<clover2_pose_msgs::srv::GetMap>(
        "~/get_map", std::bind(&server::map_callback, this,
                               std::placeholders::_1, std::placeholders::_2));

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
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

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
}

}  // namespace clover2::map

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::map::server)
