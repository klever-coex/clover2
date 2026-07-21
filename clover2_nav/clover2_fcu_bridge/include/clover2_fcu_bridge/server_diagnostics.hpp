#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace clover2_fcu_bridge {

enum class ServerDiagnostic {
    fcu_state,
    power,
    imu,
    barometer,
    interfaces,
    navigation,
};

class ServerDiagnostics
    : public clover2_common::node_interfaces::TypedNodeDiagnostics<
          ServerDiagnostic> {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(ServerDiagnostics)

    using diagnostic = ServerDiagnostic;
    using Base = clover2_common::node_interfaces::TypedNodeDiagnostics<
        diagnostic>;
    using callback = Base::callback;

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

private:
    RCLCPP_DISABLE_COPY(ServerDiagnostics)

    static const Base::DiagnosticNamesT diagnostic_names;
};

}  // namespace clover2_fcu_bridge
