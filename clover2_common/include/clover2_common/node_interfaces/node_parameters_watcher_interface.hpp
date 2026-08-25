/**
 * @file node_parameters_watcher_interface.hpp
 * @brief Provides project node parameters watcher interface.
 */

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

/**
 * @interface NodeParametersWatcherInterface
 * @brief Project node parameters watcher interface.
 */
class NodeParametersWatcherInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeParametersWatcherInterface)

    RCLCPP_PUBLIC
    virtual ~NodeParametersWatcherInterface() = default;

    using ParameterFunctorT = std::function<void(const rclcpp::Parameter&)>;

    /**
     * @brief Declare a parameter and invoke the callback when it changes.
     *
     * @param name Parameter name.
     * @param default_value Default value.
     * @param cb On change callback.
     * @param parameter_descriptor Parameter descriptor.
     * @param ignore_override Ignore parameter override.
     */
    virtual void declare_and_watch_parameter(
        const std::string& name, const rclcpp::ParameterValue& default_value,
        ParameterFunctorT cb,
        const rcl_interfaces::msg::ParameterDescriptor& parameter_descriptor =
            rcl_interfaces::msg::ParameterDescriptor(),
        bool ignore_override = false) = 0;

    /** @brief Undeclare all watched parameters. */
    RCLCPP_PUBLIC
    virtual void undeclare_watcher_parameters() = 0;
};

}  // namespace clover2_common::node_interfaces

RCLCPP_NODE_INTERFACE_HELPERS_SUPPORT(
    clover2_common::node_interfaces::NodeParametersWatcherInterface,
    parameters_watcher)
