#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/macros.hpp>
#include <rclcpp/node_interfaces/detail/node_interfaces_helpers.hpp>
#include <rclcpp/visibility_control.hpp>

// STL
#include <functional>
#include <memory>
#include <string>

namespace clover2_common::node_interfaces {

class NodeDiagnosticsInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeDiagnosticsInterface)

    using DiagnosticTaskCallbackT =
        std::function<void(diagnostic_updater::DiagnosticStatusWrapper&)>;

    RCLCPP_PUBLIC
    virtual ~NodeDiagnosticsInterface() = default;

    RCLCPP_PUBLIC
    virtual void add(const std::string& name,
                     DiagnosticTaskCallbackT callback) = 0;

    RCLCPP_PUBLIC
    virtual void remove_by_name(const std::string& name) = 0;

    RCLCPP_PUBLIC
    virtual void force_update() = 0;

    RCLCPP_PUBLIC
    virtual std::shared_ptr<diagnostic_updater::Updater> get_updater() = 0;
};

}  // namespace clover2_common::node_interfaces

RCLCPP_NODE_INTERFACE_HELPERS_SUPPORT(
    clover2_common::node_interfaces::NodeDiagnosticsInterface, diagnostics)
