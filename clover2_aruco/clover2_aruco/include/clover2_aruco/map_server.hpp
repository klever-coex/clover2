#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// Msgs includes
#include <clover2_aruco_msgs/msg/marker_map.hpp>

// Srvs includes
#include <clover2_aruco_msgs/srv/get_map.hpp>

namespace clover2_aruco {

class map_server : public rclcpp::Node {
public:
    using SharedPtr = std::shared_ptr<map_server>;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit map_server(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    void map_callback(
        const clover2_aruco_msgs::srv::GetMap::Request::SharedPtr request,
        clover2_aruco_msgs::srv::GetMap::Response::SharedPtr response);

    clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_legacy(const std::string& filename) const;
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_yaml(const std::string& filename) const;

    void update_map(clover2_aruco_msgs::msg::MarkerMap::SharedPtr map);

    void map_append_marker(clover2_aruco_msgs::msg::MarkerMap::SharedPtr& map, int id, double length, double x, double y, double z, double roll, double pitch, double yaw);

    std::string m_map_path;
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr m_map_msg;

    rclcpp::Node::OnSetParametersCallbackHandle::SharedPtr
        m_set_parameters_handle_ptr;

    rclcpp::Server<clover2_aruco_msgs::srv::GetMap>::SharedPtr m_map_server;
};

}  // namespace clover2_aruco
