#pragma once

#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_notification/output.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <string>
#include <vector>

namespace clover2_notification {

class controller : public clover2_common::lifecycle_node {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(controller)

    explicit controller(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~controller() override;

private:
    CallbackReturn on_configure(const rclcpp_lifecycle::State& state);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state);

    void diagnostics_callback(
        diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    pluginlib::ClassLoader<output> m_output_loader{
        "clover2_notification", "clover2_notification::output"};
    std::vector<std::string> m_output_plugins;
    std::vector<std::shared_ptr<output>> m_outputs;
    std::shared_ptr<clover2_common::node_context> m_node_context;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        m_diagnostics_sub;
};

}  // namespace clover2_notification
