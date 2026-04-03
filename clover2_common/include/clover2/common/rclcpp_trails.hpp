#pragma once

// ROS2
#include <rclcpp/client.hpp>
#include <rclcpp/node_interfaces/get_node_base_interface.hpp>
#include <rclcpp/node_interfaces/get_node_graph_interface.hpp>
#include <rclcpp/node_interfaces/get_node_services_interface.hpp>

namespace rclcpp {

template <typename ServiceT, typename NodeT>
typename rclcpp::Client<ServiceT>::SharedPtr create_client(
    NodeT& node, const std::string& service_name,
    const rclcpp::QoS& qos = rclcpp::ServicesQoS(),
    rclcpp::CallbackGroup::SharedPtr group = nullptr) {
    return create_client<ServiceT>(
        rclcpp::node_interfaces::get_node_base_interface(node),
        rclcpp::node_interfaces::get_node_graph_interface(node),
        rclcpp::node_interfaces::get_node_services_interface(node),
        service_name, qos.get_rmw_qos_profile(), group);
}

}  // namespace rclcpp
