#pragma once

// clover2
#include <clover2_map/data/map.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <filesystem>
#include <memory>

namespace clover2_map::io::providers {

// Format-agnostic map serialization interface.
class base_provider {
public:
    RCLCPP_DISABLE_COPY(base_provider)

    base_provider(const std::filesystem::path& filename, rclcpp::Logger logger);
    virtual ~base_provider() = default;

    virtual void load(clover2_map::map& map) = 0;
    virtual void save(const clover2_map::map& map) const = 0;

protected:
    rclcpp::Logger m_logger;
    std::filesystem::path m_filename;
};

// Creates a provider for the given file based on its extension.
// Throws std::runtime_error for unknown extensions.
std::shared_ptr<base_provider> make_provider(
    const std::filesystem::path& filename, rclcpp::Logger logger);

}  // namespace clover2_map::io::providers
