/**
 * @file sample_selector_config.hpp
 * @brief Provides sample selector configuration.
 */

#pragma once

#include <clover2_camera_utils/data/feature_values.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>

namespace clover2_camera_utils::data {

/** @brief Defines sample selection criteria for the calibration dataset. */
struct sample_selector_config {
    feature_values required_ranges{
        .x = 0.7, .y = 0.7, .size = 0.5, .tilt_x = 0.8, .tilt_y = 0.6};
    feature_values minimum_progress{
        .x = 0.7, .y = 0.7, .size = 0.9, .tilt_x = 0.9, .tilt_y = 0.9};
    double duplicate_distance_threshold{
        0.2};  ///< Maximum distance at which samples are considered duplicates.
    size_t minimum_samples_for_completion{
        20};  ///< Minimum number of samples sufficient for calibration.

    /**
     * @brief Declares and watches sample selector parameters.
     *
     * Parameter updates change this configuration. Copies made before an
     * update retain their original values.
     */
    static void declare_parameters(
        sample_selector_config& config,
        clover2_common::node_interfaces::NodeParametersWatcherInterface&
            parameters_watcher) {
        const auto declare_progress_parameter =
            [&parameters_watcher](const char* name, double& progress) {
                parameters_watcher.declare_and_watch_parameter(
                    name, rclcpp::ParameterValue(progress),
                    [&progress, name](const rclcpp::Parameter& parameter) {
                        const auto value = parameter.as_double();
                        if (value < 0.0 || value > 1.0) {
                            throw std::invalid_argument(
                                std::string(name) + " must be in [0.0, 1.0]");
                        }
                        progress = value;
                    });
            };
        declare_progress_parameter("minimum_progress_x",
                                   config.minimum_progress.x);
        declare_progress_parameter("minimum_progress_y",
                                   config.minimum_progress.y);
        declare_progress_parameter("minimum_progress_size",
                                   config.minimum_progress.size);
        declare_progress_parameter("minimum_progress_tilt_x",
                                   config.minimum_progress.tilt_x);
        declare_progress_parameter("minimum_progress_tilt_y",
                                   config.minimum_progress.tilt_y);

        parameters_watcher.declare_and_watch_parameter(
            "minimum_samples_for_completion",
            rclcpp::ParameterValue(
                static_cast<int>(config.minimum_samples_for_completion)),
            [&config](const rclcpp::Parameter& parameter) {
                const auto value = parameter.as_int();
                if (value < 3) {
                    throw std::invalid_argument(
                        "minimum_samples_for_completion must be at least 3");
                }
                config.minimum_samples_for_completion =
                    static_cast<size_t>(value);
            });
    }

    /** @brief Undeclares sample selector parameters registered by this config.
     */
    static void undeclare_parameters(
        clover2_common::node_interfaces::NodeParametersWatcherInterface&
            parameters_watcher) {
        parameters_watcher.undeclare_watcher_parameter("minimum_progress_x");
        parameters_watcher.undeclare_watcher_parameter("minimum_progress_y");
        parameters_watcher.undeclare_watcher_parameter("minimum_progress_size");
        parameters_watcher.undeclare_watcher_parameter(
            "minimum_progress_tilt_x");
        parameters_watcher.undeclare_watcher_parameter(
            "minimum_progress_tilt_y");
        parameters_watcher.undeclare_watcher_parameter(
            "minimum_samples_for_completion");
    }
};

}  // namespace clover2_camera_utils::data
