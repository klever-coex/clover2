#pragma once

#include <rclcpp/parameter_map.hpp>
#include <rclcpp/rclcpp.hpp>

namespace clover2_common::util {

using ParameterDescriptor = rcl_interfaces::msg::ParameterDescriptor;

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
    node->get_parameter(parameter_name, read_value);
}

template <typename NodeT, typename ParameterT>
void safe_declare_and_get(
    NodeT node, const std::string& parameter_name, ParameterT& read_value,
    const ParameterDescriptor& parameter_descriptor = ParameterDescriptor()) {
    safe_declare_and_get(node, parameter_name, read_value, read_value,
                         parameter_descriptor);
}

}  // namespace clover2_common::util
