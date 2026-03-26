#pragma once

#include <clover2/localization/data/sensor_data.hpp>

#include <rclcpp/rclcpp.hpp>

#include <string>

namespace clover2::localization::sensor {
struct creation_context {
    using sensor_callback = std::function<void(const data::sensor_data&)>;

    template <typename NodeT>
    explicit creation_context(NodeT node)
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
}  // namespace clover2::localization::sensor
