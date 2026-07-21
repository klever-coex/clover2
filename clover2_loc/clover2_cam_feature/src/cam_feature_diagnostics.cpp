#include <clover2/cam_feature/cam_feature_diagnostics.hpp>

namespace clover2::cam_feature {

const std::unordered_map<CamFeatureDiagnostics::diagnostic, std::string>
    CamFeatureDiagnostics::diagnostic_names = {
        {diagnostic::camera_info, "camera_info"},
        {diagnostic::map, "map"},
        {diagnostic::marker_frequency, "marker_frequency"},
};

CamFeatureDiagnostics::CamFeatureDiagnostics(
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

CamFeatureDiagnostics::~CamFeatureDiagnostics() = default;

}  // namespace clover2::cam_feature
