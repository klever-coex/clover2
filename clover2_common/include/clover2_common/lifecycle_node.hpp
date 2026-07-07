#pragma once

// clover2
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>
#include <clover2_common/util/parameter.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_common {

using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class lifecycle_node : public rclcpp_lifecycle::LifecycleNode {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(lifecycle_node)

    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    lifecycle_node(const std::string& node_name,
                   const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    virtual ~lifecycle_node();

    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        clover2_common::util::ParameterFunctorT cb,
        const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        auto parameters_watcher = get_node_parameters_watcher_interface();

        clover2_common::util::declare_and_watch_parameter<ParameterT>(
            parameters_watcher, name, default_value, cb, description,
            additional_constraints, read_only, ignore_override);
    }

    std::shared_ptr<diagnostic_updater::Updater> get_diagnostic_updater() const;

    // Custom node interfaces getters
    clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
    get_node_parameters_watcher_interface();

protected:
    void enable_diagnostic_updater();

    void produce_lifecycle_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& status);

    rclcpp::TimerBase::SharedPtr m_init_timer;

private:
    RCLCPP_DISABLE_COPY(lifecycle_node)

    std::shared_ptr<diagnostic_updater::Updater> m_diagnostic_updater;

    // Custom node interfaces
    clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
        m_parameters_watcher;
};

}  // namespace clover2_common
