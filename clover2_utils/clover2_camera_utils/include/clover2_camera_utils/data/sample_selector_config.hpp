/**
 * @file sample_selector_config.hpp
 * @brief Provides sample selector configuration.
 */

#pragma once

#include <array>
#include <cstddef>

namespace clover2_camera_utils::data {

/** @brief Defines sample selection criteria for the calibration dataset. */
struct sample_selector_config {
    std::array<double, 4> required_ranges{
        0.7, 0.7, 0.4, 0.5};  ///< Required ranges for x, y, size, and skew.
    std::array<double, 4> minimum_progress{
        0.75, 0.75, 0.8, 0.8};  ///< Minimum progress required for calibration.
    double duplicate_distance_threshold{
        0.2};  ///< Maximum distance at which samples are considered duplicates.
    size_t minimum_samples_for_completion{
        20};  ///< Minimum number of samples sufficient for calibration.
};

}  // namespace clover2_camera_utils::data
