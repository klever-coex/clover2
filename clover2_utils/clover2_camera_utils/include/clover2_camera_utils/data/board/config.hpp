/**
 * @file config.hpp
 * @brief Provides a variant of supported calibration board configurations.
 */

#pragma once

#include <clover2_camera_utils/data/board/charuco_config.hpp>
#include <clover2_camera_utils/data/board/chessboard_config.hpp>

#include <variant>

namespace clover2_camera_utils::data::board {

/** @brief Holds configuration for one supported calibration board type. */
using config = std::variant<chessboard_config, charuco_config>;

}  // namespace clover2_camera_utils::data::board
