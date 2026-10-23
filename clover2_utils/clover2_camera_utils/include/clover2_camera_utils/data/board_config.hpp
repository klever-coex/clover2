/**
 * @file board_config.hpp
 * @brief Provides calibration board configuration.
 */

#pragma once

#include <cstdint>

namespace clover2_camera_utils::data {

/** @brief Describes calibration board geometry. */
struct board_config {
    uint32_t rows{0};          ///< Number of inner corners by rows.
    uint32_t columns{0};       ///< Number of inner corners by columns.
    double square_size_m{0.0}; ///< Square side length in meters.
};

}  // namespace clover2_camera_utils::data
