/**
 * @file node_diagnostics_interface.hpp
 * @brief Provides project node diagnostics interface.
 */

#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/macros.hpp>
#include <rclcpp/node_interfaces/detail/node_interfaces_helpers.hpp>
#include <rclcpp/visibility_control.hpp>

// STL
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace clover2_common::node_interfaces {

/**
 * @interface NodeDiagnosticsInterface
 * @brief Project node diagnostics interface.
 */
class NodeDiagnosticsInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeDiagnosticsInterface)

    RCLCPP_PUBLIC
    virtual ~NodeDiagnosticsInterface() = default;

    /** @brief Add a diagnostic task. */
    RCLCPP_PUBLIC
    virtual void add(diagnostic_updater::DiagnosticTask& task) = 0;

    /**
     * @brief Add a diagnostic task of the given type.
     *
     * @tparam T Task type. Must inherit from diagnostic_updater::DiagnosticTask.
     * @tparam Args Types of task constructor arguments.
     *
     * @param args Task constructor arguments.
     */
    template <typename T, typename... Args>
    void add(Args&&... args) {
        static_assert(std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
                      "T must inherit from diagnostic_updater::DiagnosticTask");

        add_by_type(std::type_index(typeid(T)),
                    std::make_unique<T>(std::forward<Args>(args)...));
    }

    /**
     * @brief Get a diagnostic task by type.
     *
     * @tparam T Task type. Must inherit from diagnostic_updater::DiagnosticTask.
     *
     * @return Reference to the task.
     */
    template <typename T>
    T& get() {
        static_assert(std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
                      "T must inherit from diagnostic_updater::DiagnosticTask");

        return dynamic_cast<T&>(get_by_type(std::type_index(typeid(T))));
    }

    /**
     * @brief Remove a diagnostic task by type.
     *
     * @tparam T Task type. Must inherit from diagnostic_updater::DiagnosticTask.
     */
    template <typename T>
    void remove() {
        static_assert(std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
                      "T must inherit from diagnostic_updater::DiagnosticTask");

        remove_by_type(std::type_index(typeid(T)));
    }

    /** @brief Force update of all diagnostics. */
    RCLCPP_PUBLIC
    virtual void force_update() = 0;

protected:
    RCLCPP_PUBLIC
    virtual void add_by_type(
        std::type_index type,
        std::unique_ptr<diagnostic_updater::DiagnosticTask> task) = 0;

    RCLCPP_PUBLIC
    virtual diagnostic_updater::DiagnosticTask& get_by_type(
        std::type_index type) = 0;

    RCLCPP_PUBLIC
    virtual void remove_by_type(std::type_index type) = 0;
};

}  // namespace clover2_common::node_interfaces

RCLCPP_NODE_INTERFACE_HELPERS_SUPPORT(
    clover2_common::node_interfaces::NodeDiagnosticsInterface, diagnostics)
