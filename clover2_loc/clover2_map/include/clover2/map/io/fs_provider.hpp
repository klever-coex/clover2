#pragma once

// ROS2
#include <clover2_pose_msgs/msg/marker_map.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <filesystem>

namespace clover2::map::io {

class fs_provider {
public:
    RCLCPP_DISABLE_COPY(fs_provider)

    explicit fs_provider(const std::filesystem::path& filename,
                         rclcpp::Logger logger = rclcpp::get_logger("fs_provider"));
    virtual ~fs_provider() = default;

    void load();
    void save() const;

    const clover2_pose_msgs::msg::MarkerMap& get_map() const;
    clover2_pose_msgs::msg::MarkerMap& get_map();

private:
    void reset();

    rclcpp::Logger m_logger;
    std::filesystem::path m_filename;

    clover2_pose_msgs::msg::MarkerMap m_map;
};

}  // namespace clover2::map::io
