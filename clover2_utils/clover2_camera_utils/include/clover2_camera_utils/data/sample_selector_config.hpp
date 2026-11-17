/**
 * @file sample_selector_config.hpp
 * @brief Provides sample selector configuration.
 */

#pragma once

#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace clover2_camera_utils::data {

/** @brief Defines sample selection criteria for the calibration dataset. */
struct sample_selector_config {
    std::array<double, 5> required_ranges{
        0.7, 0.7, 0.5, 0.8,
        0.6};  ///< Required ranges for x, y, size, tilt_x, and tilt_y.
    std::array<double, 5> minimum_progress{
        0.7, 0.7, 0.9, 0.9,
        0.1};  ///< Minimum progress required for calibration.
    double duplicate_distance_threshold{
        0.2};  ///< Maximum distance at which samples are considered duplicates.
    size_t minimum_samples_for_completion{
        20};  ///< Minimum number of samples sufficient for calibration.

    sample_selector_config() = default;

    /**
     * @brief Declares and watches sample selector parameters.
     *
     * Parameter updates change this configuration. Copies made before an
     * update retain their original values.
     */
    sample_selector_config(
        clover2_common::node_interfaces::NodeParametersWatcherInterface&
            parameters_watcher) {
        const auto declare_progress_parameter =
            [this, &parameters_watcher](const char* name, const size_t index) {
                parameters_watcher.declare_and_watch_parameter(
                    name, rclcpp::ParameterValue(minimum_progress[index]),
                    [this, index, name](const rclcpp::Parameter& parameter) {
                        const auto value = parameter.as_double();
                        if (value < 0.0 || value > 1.0) {
                            throw std::invalid_argument(
                                std::string(name) + " must be in [0.0, 1.0]");
                        }
                        minimum_progress[index] = value;
                    });
            };
        declare_progress_parameter("minimum_progress_x", 0);
        declare_progress_parameter("minimum_progress_y", 1);
        declare_progress_parameter("minimum_progress_size", 2);
        declare_progress_parameter("minimum_progress_tilt_x", 3);
        declare_progress_parameter("minimum_progress_tilt_y", 4);

        parameters_watcher.declare_and_watch_parameter(
            "minimum_samples_for_completion",
            rclcpp::ParameterValue(
                static_cast<int>(minimum_samples_for_completion)),
            [this](const rclcpp::Parameter& parameter) {
                const auto value = parameter.as_int();
                if (value < 3) {
                    throw std::invalid_argument(
                        "minimum_samples_for_completion must be at least 3");
                }
                minimum_samples_for_completion = static_cast<size_t>(value);
            });
    }
};

}  // namespace clover2_camera_utils::data
