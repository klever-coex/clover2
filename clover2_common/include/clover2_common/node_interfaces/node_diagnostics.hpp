#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>

// ROS2
#include <rclcpp/macros.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/visibility_control.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_common::node_interfaces {

class NodeDiagnostics : public NodeDiagnosticsInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeDiagnostics)

    RCLCPP_PUBLIC
    NodeDiagnostics(
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

    RCLCPP_PUBLIC
    ~NodeDiagnostics() override;

    RCLCPP_PUBLIC
    void add(const std::string& name,
             DiagnosticTaskCallbackT callback) override;

    RCLCPP_PUBLIC
    void remove_by_name(const std::string& name) override;

    RCLCPP_PUBLIC
    void force_update() override;

private:
    RCLCPP_DISABLE_COPY(NodeDiagnostics)

    class NodeDiagnosticsImpl;
    std::unique_ptr<NodeDiagnosticsImpl> m_impl;
};

}  // namespace clover2_common::node_interfaces
