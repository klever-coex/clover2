#pragma once

// clover2
#include <clover2_offboard/helper.hpp>
#include <clover2/common/lifecycle_node.hpp>
#include <clover2/common/parameter_watcher.hpp>
#include <clover2_offboard/backend/base_backend.hpp>
#include <clover2_offboard_msgs/srv/set_position.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_offboard {

class server : public clover2::common::lifecycle_node {
public:
    using SharedPtr = std::shared_ptr<server>;
    using CallbackReturn =
        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

    explicit server(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    CallbackReturn on_configure(const rclcpp_lifecycle::State& state);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state);

private:
    void handle_set_position(
        const std::shared_ptr<clover2_offboard_msgs::srv::SetPosition::Request> request,
        std::shared_ptr<clover2_offboard_msgs::srv::SetPosition::Response> response);

    std::string m_backend_name;
    clover2::common::parameter_watcher::SharedPtr m_parameter_watcher;

    rclcpp::CallbackGroup::SharedPtr m_service_callback_group;
    rclcpp::Service<clover2_offboard_msgs::srv::SetPosition>::SharedPtr m_set_position_srv;
    std::shared_ptr<clover2_offboard::helper> m_helper;
};

}  // namespace clover2_offboard
