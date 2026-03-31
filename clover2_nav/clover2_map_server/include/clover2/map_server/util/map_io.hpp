#pragma once

#include <clover2_aruco_msgs/msg/marker_map.hpp>

#include <rclcpp/logger.hpp>

#include <filesystem>

namespace clover2::map_server::util {

class map_io {
public:
    explicit map_io(rclcpp::Logger logger = rclcpp::get_logger("map_io"));

    void load(const std::filesystem::path& filename,
              clover2_aruco_msgs::msg::MarkerMap& map);

    void save(const std::filesystem::path& filename,
              const clover2_aruco_msgs::msg::MarkerMap& map) const;

private:
    rclcpp::Logger m_logger;
};

}  // namespace clover2::map_server::util
