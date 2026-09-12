/**
 * @file temperature.hpp
 * @brief Provides a sysfs temperature notification provider.
 */

#pragma once

#include <clover2_common/node_context.hpp>
#include <clover2_notification/provider/base.hpp>

#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace clover2_notification::provider {

/**
 * @class temperature
 * @brief Periodically reads Linux sysfs temperature inputs and emits the
 * hottest value as a system notification event.
 */
class temperature final : public base {
public:
    /** @brief Provider name used by the provider factory and configuration. */
    static constexpr const char* name = "temperature";

    temperature() = default;
    ~temperature() override = default;

    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    callback_type callback) override;
    void cleanup() override;

private:
    /** @brief Read all configured inputs and emit one temperature event. */
    void update();

    /** @brief Read and convert one raw sysfs input value. */
    std::optional<double> read_temperature(const std::string& path) const;

    std::shared_ptr<clover2_common::node_context> m_node_context;
    callback_type m_callback;
    std::vector<std::string> m_paths{
        "/sys/class/thermal/thermal_zone0/temp"};
    double m_period{1.0};
    double m_scale{0.001};
    double m_warning_temperature{75.0};
    double m_error_temperature{85.0};
    int m_precision{1};
    rclcpp::TimerBase::SharedPtr m_timer;
    std::optional<rclcpp::Logger> m_logger;
};

}  // namespace clover2_notification::provider