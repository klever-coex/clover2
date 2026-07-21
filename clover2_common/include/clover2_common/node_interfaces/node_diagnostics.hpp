#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
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

template <typename DiagnosticCodeT>
class TypedNodeDiagnostics : public NodeDiagnostics {
public:
    using diagnostic = DiagnosticCodeT;
    using callback = DiagnosticTaskCallbackT;
    using DiagnosticNamesT = std::unordered_map<diagnostic, std::string>;

    RCLCPP_PUBLIC
    TypedNodeDiagnostics(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
        rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
            logging_interface,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface,
        rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
            timers_interface,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface,
        const DiagnosticNamesT& diagnostic_names)
        : NodeDiagnostics(base_interface, clock_interface, logging_interface,
                          parameters_interface, timers_interface,
                          topics_interface) {
        for (const auto& [code, name] : diagnostic_names) {
            add(name,
                [this, code](diagnostic_updater::DiagnosticStatusWrapper& stat) {
                    if (!apply_diagnostic_callback(code, stat)) {
                        stat.summary(
                            diagnostic_msgs::msg::DiagnosticStatus::STALE,
                            "Callback is not set");
                    }
                });
        }
    }

    RCLCPP_PUBLIC
    virtual ~TypedNodeDiagnostics() = default;

    RCLCPP_PUBLIC
    void set_diagnostic_callback(diagnostic diagnostic_code,
                                 callback callback) {
        m_diagnostic_callbacks[diagnostic_code] = callback;
    }

    RCLCPP_PUBLIC
    void remove_diagnostic_callback(diagnostic diagnostic_code) {
        m_diagnostic_callbacks.erase(diagnostic_code);
    }

    RCLCPP_PUBLIC
    bool apply_diagnostic_callback(
        diagnostic diagnostic_code,
        diagnostic_updater::DiagnosticStatusWrapper& status) const {
        const auto callback_it = m_diagnostic_callbacks.find(diagnostic_code);
        if (callback_it == m_diagnostic_callbacks.end()) {
            return false;
        }

        callback_it->second(status);
        return true;
    }

private:
    std::unordered_map<diagnostic, callback> m_diagnostic_callbacks;
};

}  // namespace clover2_common::node_interfaces
