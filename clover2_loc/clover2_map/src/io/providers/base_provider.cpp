#include <clover2_map/io/providers/base_provider.hpp>
#include <clover2_map/io/providers/txt.hpp>
#include <clover2_map/io/providers/yaml.hpp>

// STL
#include <stdexcept>
#include <string>

namespace clover2_map::io::providers {

base_provider::base_provider(const std::filesystem::path& filename,
                             rclcpp::Logger logger)
    : m_logger(logger)
    , m_filename(filename) {}

std::shared_ptr<base_provider> make_provider(
    const std::filesystem::path& filename, rclcpp::Logger logger) {
    const std::string ext = filename.extension().string();

    if (ext == ".txt") {
        return std::make_shared<txt>(filename, logger);
    }

    if (ext == ".yaml" || ext == ".yml") {
        return std::make_shared<yaml>(filename, logger);
    }

    throw std::runtime_error("Unknown map file extension '" + ext +
                             "' (supported: .txt, .yaml, .yml)");
}

}  // namespace clover2_map::io::providers
