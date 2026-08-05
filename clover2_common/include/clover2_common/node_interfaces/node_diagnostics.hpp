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
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>

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

    template <typename T>
    T& get() {
        static_assert(
            std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
            "T must inherit from diagnostic_updater::DiagnosticTask");

        const auto it = m_diagnostic_tasks.find(std::type_index(typeid(T)));
        if (it == m_diagnostic_tasks.end()) {
            throw std::out_of_range("Diagnostic task is not registered");
        }

        return dynamic_cast<T&>(*it->second);
    }

    RCLCPP_PUBLIC
    void remove_by_name(const std::string& name) override;

    RCLCPP_PUBLIC
    void force_update() override;

protected:
    template <typename T>
    void add() {
        static_assert(
            std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
            "T must inherit from diagnostic_updater::DiagnosticTask");

        auto task = std::make_unique<T>();
        auto& task_ref = *task;

        const auto [_, inserted] = m_diagnostic_tasks.emplace(
            std::type_index(typeid(T)), std::move(task));

        if (!inserted) {
            throw std::runtime_error("Diagnostic task is already registered");
        }

        add(task_ref);
    }

private:
    RCLCPP_DISABLE_COPY(NodeDiagnostics)

    std::shared_ptr<diagnostic_updater::Updater> m_updater;

    std::unordered_map<
        std::type_index,
        std::unique_ptr<diagnostic_updater::DiagnosticTask>> m_diagnostic_tasks;
};

}  // namespace clover2_common::node_interfaces
