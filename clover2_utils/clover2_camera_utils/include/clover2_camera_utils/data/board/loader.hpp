/**
 * @file loader.hpp
 * @brief Provides loading of calibration board configurations from JSON files.
 */

#pragma once

#include <clover2_camera_utils/data/board/config.hpp>

#include <filesystem>
#include <string_view>

namespace clover2_camera_utils::data::board {

/**
 * @brief Loads and validates a calibration board configuration from a JSON
 * file.
 * @param path Absolute path to the JSON configuration file.
 * @return Parsed board configuration.
 * @throw std::invalid_argument If the path or configuration is invalid.
 * @throw std::runtime_error If the file cannot be read or parsed.
 */
config load_config(const std::filesystem::path& path);

/**
 * @brief Returns the detector type identifier for a board configuration.
 * @param value Board configuration.
 * @return Registered detector type identifier.
 */
std::string_view type_id(const config& value) noexcept;

}  // namespace clover2_camera_utils::data::board
