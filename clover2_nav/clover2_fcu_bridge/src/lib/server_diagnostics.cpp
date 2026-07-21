#include <clover2_fcu_bridge/server_diagnostics.hpp>

namespace clover2_fcu_bridge {

const std::unordered_map<ServerDiagnostics::diagnostic, std::string>
    ServerDiagnostics::diagnostic_names = {
        {diagnostic::fcu_state, "fcu_state"},
        {diagnostic::power, "power"},
        {diagnostic::imu, "imu"},
        {diagnostic::barometer, "barometer"},
        {diagnostic::interfaces, "interfaces"},
        {diagnostic::navigation, "navigation"},
};

ServerDiagnostics::ServerDiagnostics(
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

ServerDiagnostics::~ServerDiagnostics() = default;

}  // namespace clover2_fcu_bridge
