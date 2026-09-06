#include <clover2_notification/provider/diagnostics.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

bool matches_ignore_pattern(std::string_view pattern, std::string_view value) {
    constexpr std::string_view subtree_suffix = "/*";

    if (!pattern.ends_with(subtree_suffix)) {
        return pattern == value;
    }

    pattern.remove_suffix(1);
    return value.starts_with(pattern);
}

}  // namespace

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
                "providers.diagnostics.topic",
                rclcpp::ParameterValue(
                    std::string{clover2_common::diagnostics::client::default_topic}))
            .get<std::string>();

    m_ignore_name_patterns =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter(
                "providers.diagnostics.ignore_names",
                rclcpp::ParameterValue(std::vector<std::string>{}))
            .get<std::vector<std::string>>();

    m_client = std::make_shared<clover2_common::diagnostics::client>(
        m_node_context, topic);
    m_client->set_callback(std::bind(&diagnostics::diagnostics_callback, this,
                                     std::placeholders::_1));

    RCLCPP_INFO(*m_logger, "Subscribed to diagnostics topic: %s",
                topic.c_str());
    for (const auto& pattern : m_ignore_name_patterns) {
        RCLCPP_INFO(*m_logger, "Ignoring diagnostic notifications by name: %s",
                    pattern.c_str());
    }
}

void diagnostics::cleanup() {
    if (m_client) {
        m_client->cleanup();
        m_client.reset();
    }
    m_previous.clear();
    m_ignore_name_patterns.clear();
    m_callback = nullptr;
    m_node_context.reset();
}

void diagnostics::diagnostics_callback(const message_type& msg) {
    for (const auto& status : msg.status) {
        process_status(status);
    }
}

void diagnostics::process_status(const status_type& status) {
    if (is_ignored(status)) {
        return;
    }

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

bool diagnostics::is_ignored(const status_type& status) const {
    for (const auto& pattern : m_ignore_name_patterns) {
        if (matches_ignore_pattern(pattern, status.name)) {
            RCLCPP_DEBUG(*m_logger,
                         "Ignored diagnostic notification: name='%s' "
                         "pattern='%s'",
                         status.name.c_str(), pattern.c_str());
            return true;
        }
    }

    return false;
}

}  // namespace clover2_notification::provider
