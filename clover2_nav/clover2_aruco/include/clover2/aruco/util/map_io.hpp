#pragma once

// msgs
#include <clover2_aruco_msgs/msg/marker_map.hpp>

// STL
#include <filesystem>

namespace clover2::aruco::util {

void load_from_yaml(const std::filesystem::path& filename,
                    clover2_aruco_msgs::msg::MarkerMap& map);

void load_from_txt(const std::filesystem::path& filename,
                   clover2_aruco_msgs::msg::MarkerMap& map);

}  // namespace clover2::aruco::util
