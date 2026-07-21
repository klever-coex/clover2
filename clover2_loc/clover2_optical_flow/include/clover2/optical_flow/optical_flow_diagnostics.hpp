#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace clover2::optical_flow {

enum class OpticalFlowDiagnostic {
    camera_info,
    flow,
    flow_frequency,
};

class OpticalFlowDiagnostics
    : public clover2_common::node_interfaces::TypedNodeDiagnostics<
          OpticalFlowDiagnostic> {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(OpticalFlowDiagnostics)

    using diagnostic = OpticalFlowDiagnostic;
    using Base = clover2_common::node_interfaces::TypedNodeDiagnostics<
        diagnostic>;
    using callback = Base::callback;

    OpticalFlowDiagnostics(
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

    virtual ~OpticalFlowDiagnostics();

private:
    RCLCPP_DISABLE_COPY(OpticalFlowDiagnostics)

    static const Base::DiagnosticNamesT diagnostic_names;
};

}  // namespace clover2::optical_flow
