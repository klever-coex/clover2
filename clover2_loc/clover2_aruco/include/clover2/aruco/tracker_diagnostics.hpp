#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace clover2::aruco {

class TrackerDiagnostics
    : public clover2_common::node_interfaces::NodeDiagnostics {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(TrackerDiagnostics)

    enum class diagnostic {
        map,
        markers,
        pose,
        pose_frequency,
    };

    using callback =
        std::function<void(diagnostic_updater::DiagnosticStatusWrapper&)>;

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

    void set_diagnostic_callback(diagnostic diagnostic_code, callback callback);
    void remove_diagnostic_callback(diagnostic diagnostic_code);
    bool apply_diagnostic_callback(
        diagnostic diagnostic_code,
        diagnostic_updater::DiagnosticStatusWrapper& status) const;

private:
    RCLCPP_DISABLE_COPY(TrackerDiagnostics)

    std::unordered_map<diagnostic, callback> m_diagnostic_callbacks;
    std::unordered_map<diagnostic, std::string> m_diagnostic_names;
};

}  // namespace clover2::aruco
