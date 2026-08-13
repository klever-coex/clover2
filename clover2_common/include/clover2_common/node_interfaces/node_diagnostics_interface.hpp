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

class NodeDiagnosticsInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeDiagnosticsInterface)

    RCLCPP_PUBLIC
    virtual ~NodeDiagnosticsInterface() = default;

    RCLCPP_PUBLIC
    virtual void add(diagnostic_updater::DiagnosticTask& task) = 0;

    template <typename T, typename... Args>
    void add(Args&&... args) {
        static_assert(std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
                      "T must inherit from diagnostic_updater::DiagnosticTask");

        add_by_type(std::type_index(typeid(T)),
                    std::make_unique<T>(std::forward<Args>(args)...));
    }

    template <typename T>
    T& get() {
        static_assert(std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
                      "T must inherit from diagnostic_updater::DiagnosticTask");

        return dynamic_cast<T&>(get_by_type(std::type_index(typeid(T))));
    }

    template <typename T>
    void remove() {
        static_assert(std::is_base_of_v<diagnostic_updater::DiagnosticTask, T>,
                      "T must inherit from diagnostic_updater::DiagnosticTask");

        remove_by_type(std::type_index(typeid(T)));
    }

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
