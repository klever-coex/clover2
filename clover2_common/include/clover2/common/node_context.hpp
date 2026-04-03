#pragma once

// clover2
#include <clover2/common/rclcpp_trails.hpp>

// ROS2
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_interfaces.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_time_source_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/node_interfaces/node_type_descriptions_interface.hpp>
#include <rclcpp/node_interfaces/node_waitables_interface.hpp>

namespace clover2::common {

using node_all_interfaces = rclcpp::node_interfaces::NodeInterfaces<
    rclcpp::node_interfaces::NodeBaseInterface,
    rclcpp::node_interfaces::NodeClockInterface,
    rclcpp::node_interfaces::NodeGraphInterface,
    rclcpp::node_interfaces::NodeLoggingInterface,
    rclcpp::node_interfaces::NodeParametersInterface,
    rclcpp::node_interfaces::NodeServicesInterface,
    rclcpp::node_interfaces::NodeTimeSourceInterface,
    rclcpp::node_interfaces::NodeTimersInterface,
    rclcpp::node_interfaces::NodeTopicsInterface,
    rclcpp::node_interfaces::NodeTypeDescriptionsInterface,
    rclcpp::node_interfaces::NodeWaitablesInterface>;

struct node_context : public node_all_interfaces {
    template <typename NodeT>
    node_context(NodeT& node)
        : node_all_interfaces(node) {}
};

}  // namespace clover2::common
