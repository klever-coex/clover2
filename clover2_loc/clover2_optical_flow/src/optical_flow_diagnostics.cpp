#include <clover2/optical_flow/optical_flow_diagnostics.hpp>

namespace clover2::optical_flow {

const std::unordered_map<OpticalFlowDiagnostics::diagnostic, std::string>
    OpticalFlowDiagnostics::diagnostic_names = {
        {diagnostic::camera_info, "camera_info"},
        {diagnostic::flow, "flow"},
        {diagnostic::flow_frequency, "flow_frequency"},
};

OpticalFlowDiagnostics::OpticalFlowDiagnostics(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_interface,
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        parameters_interface,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr timers_interface,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface)
    : Base(
          base_interface, clock_interface, logging_interface,
          parameters_interface, timers_interface, topics_interface,
          diagnostic_names) {}

OpticalFlowDiagnostics::~OpticalFlowDiagnostics() = default;

}  // namespace clover2::optical_flow
