/**
 * @file system.hpp
 * @brief Provides a system metrics notification provider.
 */

#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_notification/provider/base.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <optional>
#include <string>
#include <vector>

namespace clover2_notification::provider {

/**
 * @class system
 * @brief Notification provider that monitors host CPU, temperature and network
 * state.
 *
 * The provider periodically reads Linux procfs/sysfs files and emits current
 * status events with priority matching the current metric value.
 */
class system final : public base {
public:
    /** @brief Provider name used by the provider factory and configuration. */
    static constexpr const char* name = "system";

    /** @brief Construct a system provider. */
    system() = default;

    /** @brief Destroy a system provider. */
    ~system() override = default;

    /**
     * @brief Initialize provider configuration and periodic monitoring timer.
     *
     * @param node_context Shared node context used to access ROS 2 interfaces.
     * @param callback Callback invoked for generated notification events.
     */
    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    callback_type callback) override;

    /** @brief Stop monitoring and release runtime resources. */
    void cleanup() override;

private:
    struct cpu_sample {
        uint64_t idle{};
        uint64_t total{};
    };

    /** @brief Periodic monitoring callback. */
    void timer_callback();

    /** @brief Check CPU usage and emit current status event. */
    void check_cpu();

    /** @brief Check temperature and emit current status event. */
    void check_temperature();

    /** @brief Check configured network interfaces and emit status events.
     */
    void check_network();

    /** @brief Read CPU counters from configured proc stat path. */
    std::optional<cpu_sample> read_cpu_sample() const;

    /** @brief Read current CPU usage percentage using two /proc/stat samples.
     */
    std::optional<double> read_cpu_usage();

    /** @brief Read temperature from configured or first available thermal zone.
     */
    std::optional<double> read_temperature_celsius() const;

    /** @brief Read configured interface operstate. */
    std::optional<std::string> read_interface_state(
        const std::string& interface) const;

    /** @brief Read configured interface IPv4 address. */
    std::optional<std::string> read_interface_ipv4_address(
        const std::string& interface) const;

    /** @brief Read complete text file content. */
    static std::optional<std::string> read_text_file(const std::string& path);

    std::shared_ptr<clover2_common::node_context> m_node_context;
    callback_type m_callback;
    rclcpp::TimerBase::SharedPtr m_timer;
    std::optional<rclcpp::Logger> m_logger;

    bool m_enabled{false};
    double m_period{1.0};

    bool m_cpu_enabled{true};
    double m_cpu_warn_usage{85.0};
    double m_cpu_error_usage{95.0};

    bool m_temperature_enabled{true};
    double m_temperature_warn_celsius{70.0};
    double m_temperature_error_celsius{85.0};
    std::string m_temperature_zone;

    bool m_network_enabled{true};
    std::vector<std::string> m_interfaces;

    std::string m_proc_stat_path{"/proc/stat"};
    std::string m_thermal_base_path{"/sys/class/thermal"};
    std::string m_net_base_path{"/sys/class/net"};

    std::optional<cpu_sample> m_previous_cpu_sample;
};

}  // namespace clover2_notification::provider
