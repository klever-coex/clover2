#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// clover2_aruco
#include "diagnostics/map_task.hpp"

namespace clover2::aruco {

class TrackerDiagnostics
    : public clover2_common::node_interfaces::NodeDiagnostics {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(TrackerDiagnostics)

    RCLCPP_PUBLIC
    TrackerDiagnostics(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
        rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
            logging_interface,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface,
        rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
            timers_interface,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
            topics_interface)
        : NodeDiagnostics(base_interface, clock_interface, logging_interface,
                          parameters_interface, timers_interface,
                          topics_interface) {
        add<diagnostic::map>();
    }
};

}  // namespace clover2::aruco