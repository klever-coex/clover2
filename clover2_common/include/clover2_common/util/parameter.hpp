#pragma once

// clover2
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>

// ROS2
#include <rclcpp/parameter_map.hpp>
#include <rclcpp/rclcpp.hpp>

namespace clover2_common::util {

using ParameterDescriptor = rcl_interfaces::msg::ParameterDescriptor;
using ParameterFunctorT = clover2_common::node_interfaces::
    NodeParametersWatcherInterface::ParameterFunctorT;

template <typename NodeT, typename ParameterT>
void declare_parameter_if_not_declared(
    NodeT node, const std::string& parameter_name,
    const ParameterT& default_value,
    const ParameterDescriptor& parameter_descriptor = ParameterDescriptor()) {
    auto value = rclcpp::ParameterValue(default_value);

    if (!node->has_parameter(parameter_name)) {
        node->declare_parameter(parameter_name, value, parameter_descriptor);
    }
}

template <typename NodeT, typename ParameterT>
void safe_declare_and_get(
    NodeT node, const std::string& parameter_name,
    const ParameterT& default_value, ParameterT& read_value,
    const ParameterDescriptor& parameter_descriptor = ParameterDescriptor()) {
    declare_parameter_if_not_declared(node, parameter_name, default_value,
                                      parameter_descriptor);
    node->template get_parameter<ParameterT>(parameter_name, read_value);
}

template <typename NodeT, typename ParameterT>
void safe_declare_and_get(
    NodeT node, const std::string& parameter_name, ParameterT& read_value,
    const ParameterDescriptor& parameter_descriptor = ParameterDescriptor()) {
    safe_declare_and_get(node, parameter_name, read_value, read_value,
                         parameter_descriptor);
}

template <typename ParameterT, typename NodeParametersWatcherT>
void declare_and_watch_parameter(NodeParametersWatcherT& parameters_watcher,
                                 const std::string& name,
                                 const ParameterT& default_value,
                                 ParameterFunctorT cb,
                                 const std::string& description = "",
                                 const std::string& additional_constraints = "",
                                 bool read_only = false,
                                 bool ignore_override = false) {
    auto untyped_value = rclcpp::ParameterValue(default_value);

    auto descriptor = rcl_interfaces::msg::ParameterDescriptor();
    descriptor.name = name;
    descriptor.description = description;
    descriptor.additional_constraints = additional_constraints;
    descriptor.read_only = read_only;

    parameters_watcher->declare_and_watch_parameter(
        name, untyped_value, cb, descriptor, ignore_override);
}

template <typename ParameterT, typename NodeParametersWatcherT>
void declare_and_watch_parameter(
    NodeParametersWatcherT& parameters_watcher, const std::string& name,
    const ParameterT& default_value, ParameterFunctorT cb,
    const rcl_interfaces::msg::ParameterDescriptor& descriptor =
        rcl_interfaces::msg::ParameterDescriptor(),
    bool ignore_override = false) {
    declare_and_watch_parameter<NodeParametersWatcherT, ParameterT>(
        parameters_watcher, name, default_value, cb, descriptor,
        ignore_override);
}

}  // namespace clover2_common::util
