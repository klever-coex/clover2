#pragma once

// ROS2
#include "clover2/common/parameter_watcher.hpp"
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// STL
#include <memory>
#include <stdexcept>
#include <string>

namespace clover2::common {

using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class lifecycle_node : public rclcpp_lifecycle::LifecycleNode {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(lifecycle_node)

    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;
    using ParameterFunctorT = std::function<void(const rclcpp::Parameter& p)>;

    lifecycle_node(const std::string& node_name,
                   const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    virtual ~lifecycle_node();

    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        parameter_watcher::ParameterFunctorT cb,
        const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        if (!m_parameter_watcher) {
            throw std::runtime_error("Parameter watcher not created");
        }

        m_parameter_watcher->declare_and_watch_parameter(
            name, default_value, cb, description, additional_constraints,
            read_only, ignore_override);
    }

    std::shared_ptr<diagnostic_updater::Updater> get_diagnostic_updater() const;
    std::shared_ptr<parameter_watcher> get_parameter_watcher() const;

protected:
    void enable_diagnostic_updater();
    void enable_parameter_watcher();

    void produce_lifecycle_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& status);

    rclcpp::TimerBase::SharedPtr m_init_timer;

private:
    std::shared_ptr<diagnostic_updater::Updater> m_diagnostic_updater;
    std::shared_ptr<parameter_watcher> m_parameter_watcher;
};

}  // namespace clover2::common
