#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>

// ROS2
#include <rclcpp/macros.hpp>
#include <rclcpp/visibility_control.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_common::node_interfaces {

class NodeDiagnostics : public NodeDiagnosticsInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeDiagnostics)

    RCLCPP_PUBLIC
    explicit NodeDiagnostics(
        const std::shared_ptr<diagnostic_updater::Updater>& updater,
        const std::string& hardware_id);

    RCLCPP_PUBLIC
    ~NodeDiagnostics() override;

    RCLCPP_PUBLIC
    void add(const std::string& name,
             DiagnosticTaskCallbackT callback) override;

    RCLCPP_PUBLIC
    void remove_by_name(const std::string& name) override;

    RCLCPP_PUBLIC
    void force_update() override;

    RCLCPP_PUBLIC
    std::shared_ptr<diagnostic_updater::Updater> get_updater() override;

private:
    RCLCPP_DISABLE_COPY(NodeDiagnostics)

    class NodeDiagnosticsImpl;
    std::unique_ptr<NodeDiagnosticsImpl> m_impl;
};

}  // namespace clover2_common::node_interfaces
