// clover2
#include <clover2_map/io/fs_provider.hpp>
#include <clover2_map/io/providers/base_provider.hpp>

// ROS2
#include <rclcpp/logging.hpp>

namespace clover2_map::io {

fs_provider::fs_provider(const std::filesystem::path& filename,
                         rclcpp::Logger logger)
    : m_logger(logger)
    , m_filename(filename) {}

void fs_provider::load() {
    reset();

    RCLCPP_DEBUG(m_logger, "Map format by extension: %s (%s)",
                 m_filename.extension().string().c_str(), m_filename.c_str());

    auto p = providers::make_provider(m_filename, m_logger);
    p->load(m_map);
}

void fs_provider::save() const {
    auto p = providers::make_provider(m_filename, m_logger);
    p->save(m_map);
}

const clover2_map::map& fs_provider::get_map() const { return m_map; }

clover2_map::map& fs_provider::get_map() { return m_map; }

void fs_provider::reset() { m_map = clover2_map::map{}; }

}  // namespace clover2_map::io
