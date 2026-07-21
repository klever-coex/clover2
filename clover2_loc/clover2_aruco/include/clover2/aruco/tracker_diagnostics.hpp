#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace clover2::aruco {

enum class TrackerDiagnostic {
    map,
    markers,
    pose,
    pose_frequency,
};

class TrackerDiagnostics
    : public clover2_common::node_interfaces::TypedNodeDiagnostics<
          TrackerDiagnostic> {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(TrackerDiagnostics)

    using diagnostic = TrackerDiagnostic;
    using Base = clover2_common::node_interfaces::TypedNodeDiagnostics<
        diagnostic>;
    using callback = Base::callback;

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
            topics_interface);

    virtual ~TrackerDiagnostics();

private:
    RCLCPP_DISABLE_COPY(TrackerDiagnostics)

    static const Base::DiagnosticNamesT diagnostic_names;
};

}  // namespace clover2::aruco
