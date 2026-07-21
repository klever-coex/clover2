#include <clover2/map/server_diagnostics.hpp>

namespace clover2::map {

const std::unordered_map<MapServerDiagnostics::diagnostic, std::string>
    MapServerDiagnostics::diagnostic_names = {
        {diagnostic::map, "map"},
        {diagnostic::interface, "interface"},
};

MapServerDiagnostics::MapServerDiagnostics(
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

MapServerDiagnostics::~MapServerDiagnostics() = default;

}  // namespace clover2::map
