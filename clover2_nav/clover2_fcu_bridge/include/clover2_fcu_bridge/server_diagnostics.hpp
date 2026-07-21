#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace clover2_fcu_bridge {

class ServerDiagnostics
    : public clover2_common::node_interfaces::NodeDiagnostics {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(ServerDiagnostics)

    enum class diagnostic {
        fcu_state,
        power,
        imu,
        barometer,
        interfaces,
        navigation,
    };

    using callback =
        std::function<void(diagnostic_updater::DiagnosticStatusWrapper&)>;

    ServerDiagnostics(
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

    virtual ~ServerDiagnostics();

    void set_diagnostic_callback(diagnostic diagnostic_code, callback callback);
    void remove_diagnostic_callback(diagnostic diagnostic_code);
    bool apply_diagnostic_callback(
        diagnostic diagnostic_code,
        diagnostic_updater::DiagnosticStatusWrapper& status) const;

private:
    RCLCPP_DISABLE_COPY(ServerDiagnostics)

    static const std::unordered_map<diagnostic, std::string> diagnostic_names;

    std::unordered_map<diagnostic, callback> m_diagnostic_callbacks;
};

}  // namespace clover2_fcu_bridge