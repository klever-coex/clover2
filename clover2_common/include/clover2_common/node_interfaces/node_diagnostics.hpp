#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
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
#include <typeindex>
#include <unordered_map>

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
    void add(diagnostic_updater::DiagnosticTask& task) override;

    RCLCPP_PUBLIC
    void remove_by_name(const std::string& name) override;

    RCLCPP_PUBLIC
    void force_update() override;

protected:
    RCLCPP_PUBLIC
    void add_by_type(
        std::type_index type,
        std::unique_ptr<diagnostic_updater::DiagnosticTask> task) override;

    RCLCPP_PUBLIC
    diagnostic_updater::DiagnosticTask& get_by_type(
        std::type_index type) override;

private:
    RCLCPP_DISABLE_COPY(NodeDiagnostics)

    std::shared_ptr<diagnostic_updater::Updater> m_updater;

    std::unordered_map<std::type_index,
                       std::unique_ptr<diagnostic_updater::DiagnosticTask>>
        m_diagnostic_tasks;
};

}  // namespace clover2_common::node_interfaces
