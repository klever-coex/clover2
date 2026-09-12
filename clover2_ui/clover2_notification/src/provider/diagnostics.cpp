#include <clover2_notification/provider/diagnostics.hpp>
#include <clover2_common/util/parameter.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <functional>
#include <memory>
#include <optional>
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

std::optional<std::string> find_diagnostic_value(
    const diagnostic_msgs::msg::DiagnosticStatus& status,
    std::string_view key) {
    for (const auto& value : status.values) {
        if (value.key == key) {
            return value.value;
        }
    }

    return std::nullopt;
}

std::optional<double> parse_number(std::string_view value) {
    try {
        size_t parsed{};
        const auto result = std::stod(std::string(value), &parsed);
        if (parsed == value.size()) {
            return result;
        }
    } catch (const std::exception&) {
    }

    return std::nullopt;
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

    const auto parameters = m_node_context->get_node_parameters_interface();
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.diagnostics.topic", m_topic);
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.diagnostics.ignore_names", m_ignore_name_patterns);

    m_client = std::make_shared<clover2_common::diagnostics::client>(
        m_node_context, m_topic);
    m_client->set_callback(std::bind(&diagnostics::diagnostics_callback, this,
                                     std::placeholders::_1));

    RCLCPP_INFO(*m_logger, "Subscribed to diagnostics topic: %s",
                m_topic.c_str());
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
    m_callback = nullptr;
    m_node_context.reset();
}

void diagnostics::diagnostics_callback(const message_type& msg) {
    for (const auto& status : msg.status) {
        process_status(status);
    }
}

void diagnostics::process_status(const status_type& status) {
    process_system_status(status);

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

void diagnostics::process_system_status(const status_type& status) {
    if (status.name.ends_with("CPU Information")) {
        const auto cpu_load = find_diagnostic_value(status, "CPU Load Average");
        if (cpu_load) {
            m_callback({static_cast<int>(status.level), "system", "cpu",
                        *cpu_load});
        }
        return;
    }

    if (!status.name.ends_with("Sensor Status")) {
        return;
    }

    std::optional<std::pair<double, std::string>> maximum_temperature;
    for (const auto& value : status.values) {
        if (!std::string_view(value.key).ends_with(" Temperature")) {
            continue;
        }

        const auto temperature = parse_number(value.value);
        if (temperature && (!maximum_temperature ||
                            *temperature > maximum_temperature->first)) {
            maximum_temperature = std::make_pair(*temperature, value.value);
        }
    }

    if (maximum_temperature) {
        m_callback({static_cast<int>(status.level), "system", "temperature",
                    maximum_temperature->second});
    }
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
