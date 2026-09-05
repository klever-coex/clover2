#include <arpa/inet.h>
#include <clover2_notification/provider/system.hpp>
#include <netinet/in.h>
#include <rclcpp/create_timer.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <ifaddrs.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

constexpr int k_ok_priority = 0;
constexpr int k_warn_priority = 1;
constexpr int k_error_priority = 2;

std::string trim(std::string value) {
    value.erase(value.begin(),
                std::find_if(value.begin(), value.end(),
                             [](unsigned char c) { return !std::isspace(c); }));
    value.erase(std::find_if(value.rbegin(), value.rend(),
                             [](unsigned char c) { return !std::isspace(c); })
                    .base(),
                value.end());
    return value;
}

}  // namespace

namespace clover2_notification::provider {

void system::initialize(
    std::shared_ptr<clover2_common::node_context> node_context,
    callback_type callback) {
    if (!node_context) {
        throw std::invalid_argument("System provider received null context");
    }
    if (!callback) {
        throw std::invalid_argument("System provider received empty callback");
    }

    m_node_context = std::move(node_context);
    m_callback = std::move(callback);
    m_previous_cpu_sample.reset();
    m_logger = m_node_context->get_logger().get_child("system_provider");

    m_enabled =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.enabled",
                                rclcpp::ParameterValue(m_enabled))
            .get<bool>();
    m_period =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.period",
                                rclcpp::ParameterValue(m_period))
            .get<double>();

    m_cpu_enabled =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.cpu.enabled",
                                rclcpp::ParameterValue(m_cpu_enabled))
            .get<bool>();
    m_cpu_warn_usage =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.cpu.warn_usage",
                                rclcpp::ParameterValue(m_cpu_warn_usage))
            .get<double>();
    m_cpu_error_usage =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.cpu.error_usage",
                                rclcpp::ParameterValue(m_cpu_error_usage))
            .get<double>();

    m_temperature_enabled =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.temperature.enabled",
                                rclcpp::ParameterValue(m_temperature_enabled))
            .get<bool>();
    m_temperature_warn_celsius =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter(
                "providers.system.temperature.warn_celsius",
                rclcpp::ParameterValue(m_temperature_warn_celsius))
            .get<double>();
    m_temperature_error_celsius =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter(
                "providers.system.temperature.error_celsius",
                rclcpp::ParameterValue(m_temperature_error_celsius))
            .get<double>();
    m_temperature_zone =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.temperature.thermal_zone",
                                rclcpp::ParameterValue(m_temperature_zone))
            .get<std::string>();

    m_network_enabled =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.network.enabled",
                                rclcpp::ParameterValue(m_network_enabled))
            .get<bool>();
    m_interfaces =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.network.interfaces",
                                rclcpp::ParameterValue(m_interfaces))
            .get<std::vector<std::string>>();

    m_proc_stat_path =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.proc_stat_path",
                                rclcpp::ParameterValue(m_proc_stat_path))
            .get<std::string>();
    m_thermal_base_path =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.thermal_base_path",
                                rclcpp::ParameterValue(m_thermal_base_path))
            .get<std::string>();
    m_net_base_path =
        rclcpp::node_interfaces::get_node_parameters_interface(m_node_context)
            ->declare_parameter("providers.system.net_base_path",
                                rclcpp::ParameterValue(m_net_base_path))
            .get<std::string>();

    if (m_period <= 0.0) {
        throw std::invalid_argument(
            "System provider period should be positive");
    }
    if (m_cpu_warn_usage > m_cpu_error_usage) {
        throw std::invalid_argument(
            "System provider CPU warn_usage should not exceed error_usage");
    }
    if (m_temperature_warn_celsius > m_temperature_error_celsius) {
        throw std::invalid_argument(
            "System provider temperature warn_celsius should not exceed "
            "error_celsius");
    }

    if (!m_enabled) {
        RCLCPP_INFO(*m_logger, "System provider is disabled");
        return;
    }

    const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(m_period));
    m_timer = rclcpp::create_timer(
        m_node_context->get_node_base_interface(),
        m_node_context->get_node_timers_interface(),
        m_node_context->get_node_clock_interface()->get_clock(), period,
        [this]() { timer_callback(); });

    RCLCPP_INFO(*m_logger,
                "System provider enabled: period=%.2fs cpu=%s temperature=%s "
                "network=%s",
                m_period, m_cpu_enabled ? "enabled" : "disabled",
                m_temperature_enabled ? "enabled" : "disabled",
                m_network_enabled ? "enabled" : "disabled");

    timer_callback();
}

void system::cleanup() {
    if (m_timer) {
        m_timer->cancel();
        m_timer.reset();
    }
    m_previous_cpu_sample.reset();
    m_callback = nullptr;
    m_node_context.reset();
}

void system::timer_callback() {
    if (m_cpu_enabled) {
        check_cpu();
    }
    if (m_temperature_enabled) {
        check_temperature();
    }
    if (m_network_enabled) {
        check_network();
    }
}

void system::check_cpu() {
    const auto usage = read_cpu_usage();
    if (!usage) {
        RCLCPP_WARN_THROTTLE(
            *m_logger, *m_node_context->get_node_clock_interface()->get_clock(),
            30000, "Failed to read CPU usage from %s",
            m_proc_stat_path.c_str());
        return;
    }

    int priority = k_ok_priority;
    if (*usage >= m_cpu_error_usage) {
        priority = k_error_priority;
    } else if (*usage >= m_cpu_warn_usage) {
        priority = k_warn_priority;
    }

    m_callback({priority, "system", "cpu", std::format("{:.1f}", *usage)});
}

void system::check_temperature() {
    const auto temperature = read_temperature_celsius();
    if (!temperature) {
        RCLCPP_WARN_THROTTLE(
            *m_logger, *m_node_context->get_node_clock_interface()->get_clock(),
            30000, "Failed to read CPU temperature from %s",
            m_thermal_base_path.c_str());
        return;
    }

    int priority = k_ok_priority;
    if (*temperature >= m_temperature_error_celsius) {
        priority = k_error_priority;
    } else if (*temperature >= m_temperature_warn_celsius) {
        priority = k_warn_priority;
    }

    m_callback({priority, "system", "temperature",
                std::format("{:.1f}", *temperature)});
}

void system::check_network() {
    std::string selected_interface;
    std::optional<std::string> selected_ip_address;
    bool selected_up{};
    int selected_score{-1};

    for (const auto& interface : m_interfaces) {
        const auto state = read_interface_state(interface);
        const bool up = state && *state == "up";
        const auto ip_address = read_interface_ipv4_address(interface);

        const int score = (up ? 2 : 0) + (ip_address ? 1 : 0);
        if (score <= selected_score) {
            continue;
        }

        selected_interface = interface;
        selected_ip_address = ip_address;
        selected_up = up;
        selected_score = score;
    }

    if (selected_interface.empty()) {
        return;
    }

    const int priority = selected_up ? k_ok_priority : k_warn_priority;
    const std::string status_message =
        selected_interface + " " +
        (selected_ip_address ? *selected_ip_address : std::string{"-"});

    m_callback({priority, "system", "network", status_message});
}

std::optional<system::cpu_sample> system::read_cpu_sample() const {
    const auto content = read_text_file(m_proc_stat_path);
    if (!content) {
        return std::nullopt;
    }

    std::istringstream stream(*content);
    std::string cpu;
    uint64_t user{};
    uint64_t nice{};
    uint64_t system_time{};
    uint64_t idle{};
    uint64_t iowait{};
    uint64_t irq{};
    uint64_t softirq{};
    uint64_t steal{};

    stream >> cpu >> user >> nice >> system_time >> idle >> iowait >> irq >>
        softirq >> steal;
    if (!stream || cpu != "cpu") {
        return std::nullopt;
    }

    cpu_sample sample;
    sample.idle = idle + iowait;
    sample.total =
        user + nice + system_time + idle + iowait + irq + softirq + steal;
    return sample;
}

std::optional<double> system::read_cpu_usage() {
    const auto current = read_cpu_sample();
    if (!current) {
        return std::nullopt;
    }

    if (!m_previous_cpu_sample) {
        m_previous_cpu_sample = current;
        return 0.0;
    }

    const auto previous = *m_previous_cpu_sample;
    m_previous_cpu_sample = current;

    const uint64_t total_delta = current->total - previous.total;
    const uint64_t idle_delta = current->idle - previous.idle;
    if (total_delta == 0 || idle_delta > total_delta) {
        return std::nullopt;
    }

    return 100.0 * static_cast<double>(total_delta - idle_delta) /
           static_cast<double>(total_delta);
}

std::optional<double> system::read_temperature_celsius() const {
    std::vector<std::filesystem::path> candidates;
    if (!m_temperature_zone.empty()) {
        candidates.emplace_back(std::filesystem::path(m_thermal_base_path) /
                                m_temperature_zone / "temp");
    } else {
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(
                 m_thermal_base_path,
                 std::filesystem::directory_options::skip_permission_denied,
                 ec)) {
            if (ec) {
                break;
            }
            if (!entry.is_directory(ec)) {
                continue;
            }

            const auto filename = entry.path().filename().string();
            if (filename.starts_with("thermal_zone")) {
                candidates.emplace_back(entry.path() / "temp");
            }
        }
    }

    std::sort(candidates.begin(), candidates.end());
    for (const auto& path : candidates) {
        const auto content = read_text_file(path.string());
        if (!content) {
            continue;
        }

        try {
            return std::stod(trim(*content)) / 1000.0;
        } catch (const std::exception&) {
        }
    }

    return std::nullopt;
}

std::optional<std::string> system::read_interface_state(
    const std::string& interface) const {
    const auto path =
        (std::filesystem::path(m_net_base_path) / interface / "operstate")
            .string();
    const auto content = read_text_file(path);
    if (!content) {
        return std::nullopt;
    }

    return trim(*content);
}

std::optional<std::string> system::read_interface_ipv4_address(
    const std::string& interface) const {
    const auto fake_path =
        (std::filesystem::path(m_net_base_path) / interface / "ipv4_address")
            .string();
    const auto fake_content = read_text_file(fake_path);
    if (fake_content) {
        const auto ip_address = trim(*fake_content);
        if (!ip_address.empty()) {
            return ip_address;
        }
    }

    ifaddrs* interfaces{};
    if (getifaddrs(&interfaces) != 0) {
        return std::nullopt;
    }

    std::optional<std::string> result;
    for (auto* current = interfaces; current != nullptr;
         current = current->ifa_next) {
        if (current->ifa_addr == nullptr || current->ifa_name == nullptr) {
            continue;
        }
        if (interface != current->ifa_name) {
            continue;
        }
        if (current->ifa_addr->sa_family != AF_INET) {
            continue;
        }

        char buffer[INET_ADDRSTRLEN]{};
        const auto* address = reinterpret_cast<sockaddr_in*>(current->ifa_addr);
        if (inet_ntop(AF_INET, &address->sin_addr, buffer, sizeof(buffer)) !=
            nullptr) {
            result = buffer;
            break;
        }
    }

    freeifaddrs(interfaces);
    return result;
}

std::optional<std::string> system::read_text_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return std::nullopt;
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

}  // namespace clover2_notification::provider
