#include <clover2/aruco/tracker_diagnostics.hpp>

namespace clover2::aruco {

const std::unordered_map<TrackerDiagnostics::diagnostic, std::string>
    TrackerDiagnostics::diagnostic_names = {
        {diagnostic::map, "map"},
        {diagnostic::markers, "markers"},
        {diagnostic::pose, "pose"},
        {diagnostic::pose_frequency, "pose_frequency"},
};

TrackerDiagnostics::TrackerDiagnostics(
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

TrackerDiagnostics::~TrackerDiagnostics() = default;

}  // namespace clover2::aruco
