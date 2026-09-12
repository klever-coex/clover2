#include <clover2_notification/data/priority.hpp>
#include <clover2_notification/provider/temperature.hpp>

#include <clover2_common/util/parameter.hpp>
#include <clover2_common/util/timer.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace clover2_notification::provider {

void temperature::initialize(
    std::shared_ptr<clover2_common::node_context> node_context,
    callback_type callback) {
    if (!node_context) {
        throw std::invalid_argument("Temperature provider received null context");
    }
    if (!callback) {
        throw std::invalid_argument("Temperature provider received empty callback");
    }

    m_node_context = std::move(node_context);
    m_callback = std::move(callback);
    m_logger = m_node_context->get_logger().get_child("temperature_provider");
    const auto parameters = m_node_context->get_node_parameters_interface();
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.temperature.paths", m_paths);
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.temperature.period", m_period);
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.temperature.scale", m_scale);
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.temperature.warning_temperature",
        m_warning_temperature);
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.temperature.error_temperature",
        m_error_temperature);
    clover2_common::util::safe_declare_and_get(
        parameters, "providers.temperature.precision", m_precision);

    if (m_paths.empty() ||
        std::any_of(m_paths.begin(), m_paths.end(),
                    [](const auto& path) { return path.empty(); }) ||
        m_period <= 0.0 || m_scale <= 0.0 ||
        m_warning_temperature > m_error_temperature || m_precision < 0) {
        throw std::invalid_argument("Invalid temperature provider configuration");
    }

    const auto timer_period =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(m_period));
    m_timer = clover2_common::util::create_timer(
        m_node_context, timer_period, [this]() { update(); });

    RCLCPP_INFO(*m_logger,
                "Temperature provider initialized: inputs=%zu period=%.2fs",
                m_paths.size(), m_period);
}

void temperature::cleanup() {
    if (m_timer) {
        m_timer->cancel();
        m_timer.reset();
    }
    m_callback = nullptr;
    m_node_context.reset();
    m_logger.reset();
}

void temperature::update() {
    std::optional<double> maximum_temperature;
    for (const auto& path : m_paths) {
        const auto value = read_temperature(path);
        if (value && (!maximum_temperature || *value > *maximum_temperature)) {
            maximum_temperature = value;
        }
    }

    if (!maximum_temperature) {
        RCLCPP_WARN(*m_logger, "Unable to read configured temperature inputs");
        m_callback({static_cast<int>(data::priority::stale), "system",
                    "temperature", "n/a"});
        return;
    }

    auto priority = data::priority::ok;
    if (*maximum_temperature >= m_error_temperature) {
        priority = data::priority::error;
    } else if (*maximum_temperature >= m_warning_temperature) {
        priority = data::priority::warning;
    }

    std::ostringstream message;
    message << std::fixed << std::setprecision(m_precision)
            << *maximum_temperature;
    m_callback({static_cast<int>(priority), "system", "temperature",
                message.str()});
}

std::optional<double> temperature::read_temperature(
    const std::string& path) const {
    std::ifstream input(path);
    std::string value;
    if (!input || !std::getline(input, value)) {
        RCLCPP_DEBUG(*m_logger, "Unable to read temperature input: %s",
                     path.c_str());
        return std::nullopt;
    }

    try {
        size_t parsed{};
        const auto raw = std::stod(value, &parsed);
        if (parsed != value.find_last_not_of(" \t\r\n") + 1) {
            throw std::invalid_argument("trailing characters");
        }
        return raw * m_scale;
    } catch (const std::exception&) {
        RCLCPP_WARN(*m_logger, "Invalid temperature input '%s': %s",
                    path.c_str(), value.c_str());
        return std::nullopt;
    }
}

}  // namespace clover2_notification::provider