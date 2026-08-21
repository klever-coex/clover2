#include <clover2_notification/provider/diagnostics.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <functional>
#include <stdexcept>
#include <utility>

namespace clover2_notification::provider {

void diagnostics::initialize(
    std::shared_ptr<clover2_common::node_context> node_context,
    callback_type callback) {
    if (!node_context) {
        throw std::invalid_argument(
            "Diagnostics provider received null context");
    }
    if (!callback) {
        throw std::invalid_argument(
            "Diagnostics provider received empty callback");
    }

    m_node_context = std::move(node_context);
    m_callback = std::move(callback);
    m_previous.clear();
    m_logger = m_node_context->get_logger().get_child("diagnostics_provider");

    const auto topic =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter(
                "diagnostics.topic",
                rclcpp::ParameterValue(std::string{"/diagnostics_agg"}))
            .get<std::string>();

    m_client.initialize(m_node_context, topic,
                        std::bind(&diagnostics::diagnostics_callback, this,
                                  std::placeholders::_1));

    RCLCPP_INFO(*m_logger, "Subscribed to diagnostics topic: %s",
                topic.c_str());
}

void diagnostics::cleanup() {
    m_client.cleanup();
    m_previous.clear();
    m_callback = nullptr;
    m_node_context.reset();
}

void diagnostics::diagnostics_callback(const status_type& status) {
    const auto previous_it = m_previous.find(status.name);
    const bool changed = previous_it == m_previous.end() ||
                         clover2_common::diagnostics::client::status_changed(
                             previous_it->second, status);
    m_previous.insert_or_assign(status.name, status);

    if (!changed ||
        status.level == diagnostic_msgs::msg::DiagnosticStatus::OK) {
        return;
    }

    m_callback({static_cast<int>(status.level), "diagnostics", status.name,
                status.message});
}

}  // namespace clover2_notification::provider
