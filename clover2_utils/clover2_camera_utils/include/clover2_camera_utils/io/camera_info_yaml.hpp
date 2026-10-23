/**
 * @file camera_info_yaml.hpp
 * @brief Provides serialization of camera information to a YAML file.
 */

#pragma once

#include <sensor_msgs/msg/camera_info.hpp>

#include <filesystem>
#include <string>

namespace clover2_camera_utils::io {

/**
 * @brief Saves camera information in the standard ROS camera-info YAML format.
 * @param camera_info Camera parameters to save.
 * @param path Destination YAML file path.
 * @param error Output reason when the file cannot be written.
 * @return `true` on success, otherwise `false`.
 */
bool save_camera_info_yaml(const sensor_msgs::msg::CameraInfo& camera_info,
                           const std::filesystem::path& path,
                           std::string& error);

}  // namespace clover2_camera_utils::io