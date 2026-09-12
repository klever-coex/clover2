#pragma once

// ROS2
#include <rclcpp/create_timer.hpp>
#include <rclcpp/duration.hpp>
#include <rclcpp/node_interfaces/get_node_clock_interface.hpp>

// STL
#include <utility>

namespace clover2_common::util {

template <typename NodeT, typename DurationT, typename CallbackT>
rclcpp::TimerBase::SharedPtr create_timer(NodeT node, DurationT period,
                                          CallbackT&& callback) {
    return rclcpp::create_timer(
        node, rclcpp::node_interfaces::get_node_clock_interface(node)->get_clock(),
        rclcpp::Duration(period), std::forward<CallbackT>(callback));
}

}  // namespace clover2_common::util