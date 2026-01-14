#pragma once

#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// STL
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace clover2_common {

using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class lifecycle_node : public rclcpp_lifecycle::LifecycleNode {
public:
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;
    using ParameterFunctorT = std::function<void(const rclcpp::Parameter& p)>;
    using SharedPtr = std::shared_ptr<rclcpp_lifecycle::LifecycleNode>;

    lifecycle_node(const std::string& node_name,
                   const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    virtual ~lifecycle_node();

    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        ParameterFunctorT cb, const std::string& description = "",
        const std::string& additional_constraints = "",
        bool read_only = false) {
        m_watch_parameters[name] = cb;

        auto descriptor = rcl_interfaces::msg::ParameterDescriptor();

        descriptor.name = name;
        descriptor.description = description;
        descriptor.additional_constraints = additional_constraints;
        descriptor.read_only = read_only;

        declare_parameter<ParameterT>(descriptor.name, default_value,
                                      descriptor);
    }

    void enable_watch_parameters();
    void enable_diagnostic_updater();

protected:
    SetParametersResult on_set_parameters_cb(
        const std::vector<rclcpp::Parameter>& parameters);

    void produce_lifecycle_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& status);

    rclcpp::Node::OnSetParametersCallbackHandle::SharedPtr
        m_set_parameters_handle_ptr;

    rclcpp::TimerBase::SharedPtr m_init_timer;

    std::shared_ptr<diagnostic_updater::Updater> m_diagnostic_updater;
    std::unordered_map<std::string, ParameterFunctorT> m_watch_parameters;
};

}  // namespace clover2_common
