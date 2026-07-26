#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>

// ROS2
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>

// STL
#include <memory>
#include <type_traits>

namespace clover2_common {

template <typename DiagnosticsT = node_interfaces::NodeDiagnostics>
class NodeInterfacesFactory {
public:
    NodeInterfacesFactory() = default;
    ~NodeInterfacesFactory() = default;

    static node_interfaces::NodeDiagnosticsInterface::SharedPtr
    create_diagnostics(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
        rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
            logging_interface,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface,
        rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
            timers_interface,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
            topics_interface) {
        static_assert(
            std::is_base_of_v<node_interfaces::NodeDiagnosticsInterface,
                              DiagnosticsT>,
            "DiagnosticsT must implement NodeDiagnosticsInterface");

        return std::make_shared<DiagnosticsT>(
            base_interface, clock_interface, logging_interface,
            parameters_interface, timers_interface, topics_interface);
    }

    static node_interfaces::NodeParametersWatcherInterface::SharedPtr
    create_parameters_watcher(
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface) {
        return std::make_shared<node_interfaces::NodeParametersWatcher>(
            parameters_interface);
    }
};

}  // namespace clover2_common
