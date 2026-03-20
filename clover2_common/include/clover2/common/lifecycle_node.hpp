#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// STL
#include <memory>
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

    void enable_diagnostic_updater();

    std::shared_ptr<diagnostic_updater::Updater> get_diagnostic_updater();

protected:
    void produce_lifecycle_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& status);

    rclcpp::TimerBase::SharedPtr m_init_timer;

    std::shared_ptr<diagnostic_updater::Updater> m_diagnostic_updater;
};

}  // namespace clover2::common
