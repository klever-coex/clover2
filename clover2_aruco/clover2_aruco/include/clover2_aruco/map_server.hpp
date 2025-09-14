#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// Msgs includes
#include <clover2_aruco_msgs/msg/marker_map.hpp>

// Srvs includes
#include <clover2_aruco_msgs/srv/get_map.hpp>

namespace clover2_aruco {

class map_server : public rclcpp_lifecycle::LifecycleNode {
public:
    using SharedPtr = std::shared_ptr<map_server>;
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::
        LifecycleNodeInterface::CallbackReturn;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit map_server(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    CallbackReturn on_configure(const rclcpp_lifecycle::State& state);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state);

private:
    void map_callback(
        const clover2_aruco_msgs::srv::GetMap::Request::SharedPtr request,
        clover2_aruco_msgs::srv::GetMap::Response::SharedPtr response);

    std::string m_map_path;
    clover2_aruco_msgs::msg::MarkerMap m_map_msg;

    rclcpp::Node::OnSetParametersCallbackHandle::SharedPtr
        m_set_parameters_handle_ptr;

    rclcpp::Server<clover2_aruco_msgs::srv::GetMap>::SharedPtr m_map_server;
};

}  // namespace clover2_aruco
