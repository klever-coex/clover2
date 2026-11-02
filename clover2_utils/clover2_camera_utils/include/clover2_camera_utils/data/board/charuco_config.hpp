/**
 * @file charuco_config.hpp
 * @brief Provides configuration for a ChArUco calibration target.
 */

#pragma once

#include <cstdint>
#include <string>

namespace clover2_camera_utils::data::board {

/** @brief Describes the geometry and dictionary of a ChArUco target. */
struct charuco_config {
    uint32_t squares_rows{0};     ///< Number of squares by rows.
    uint32_t squares_columns{0};  ///< Number of squares by columns.
    double square_size_m{0.0};    ///< Square side length in meters.
    double marker_size_m{0.0};    ///< ArUco marker side length in meters.
    std::string dictionary{"DICT_4X4_50"};  ///< Predefined ArUco dictionary.
};

}  // namespace clover2_camera_utils::data::board
