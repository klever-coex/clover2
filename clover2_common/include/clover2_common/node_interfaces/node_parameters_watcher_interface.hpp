#pragma once

// ROS2
#include <rcl_interfaces/msg/parameter_descriptor.hpp>
#include <rclcpp/macros.hpp>
#include <rclcpp/node_interfaces/detail/node_interfaces_helpers.hpp>
#include <rclcpp/parameter.hpp>
#include <rclcpp/visibility_control.hpp>

// STL
#include <functional>

namespace clover2_common::node_interfaces {

class NodeParametersWatcherInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeParametersWatcherInterface)

    RCLCPP_PUBLIC
    virtual ~NodeParametersWatcherInterface() = default;

    using ParameterFunctorT = std::function<void(const rclcpp::Parameter&)>;

    virtual void declare_and_watch_parameter(
        const std::string& name, const rclcpp::ParameterValue& default_value,
        ParameterFunctorT cb,
        const rcl_interfaces::msg::ParameterDescriptor& parameter_descriptor =
            rcl_interfaces::msg::ParameterDescriptor(),
        bool ignore_override = false) = 0;

    RCLCPP_PUBLIC
    virtual void undeclare_watcher_parameters() = 0;
};

}  // namespace clover2_common::node_interfaces

RCLCPP_NODE_INTERFACE_HELPERS_SUPPORT(
    clover2_common::node_interfaces::NodeParametersWatcherInterface,
    parameters_watcher)
