#pragma once

#include <rclcpp/rclcpp.hpp>

#include <string>

namespace clover2::common {

struct node_context {
    template <typename NodeT>
    explicit node_context(NodeT node)
        : node_base(node.get_node_base_interface())
        , node_logging(node.get_node_logging_interface())
        , node_parameters(node.get_node_parameters_interface())
        , node_timers(node.get_node_timers_interface()) {}

    std::shared_ptr<rclcpp::node_interfaces::NodeBaseInterface> node_base;
    std::shared_ptr<rclcpp::node_interfaces::NodeLoggingInterface> node_logging;
    std::shared_ptr<rclcpp::node_interfaces::NodeParametersInterface>
        node_parameters;
    std::shared_ptr<rclcpp::node_interfaces::NodeTimersInterface> node_timers;
};

}  // namespace clover2::common
