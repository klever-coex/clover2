#pragma once

// clover2
#include <clover2_map/data/map.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <filesystem>

namespace clover2_map::io {

class fs_provider {
public:
    RCLCPP_DISABLE_COPY(fs_provider)

    explicit fs_provider(
        const std::filesystem::path& filename,
        rclcpp::Logger logger = rclcpp::get_logger("fs_provider"));
    virtual ~fs_provider() = default;

    void load();
    void save() const;

    const clover2_map::map& get_map() const;
    clover2_map::map& get_map();

private:
    void reset();

    rclcpp::Logger m_logger;
    std::filesystem::path m_filename;

    clover2_map::map m_map;
};

}  // namespace clover2_map::io
