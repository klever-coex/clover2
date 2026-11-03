#include <clover2_camera_utils/calibration/target/charuco.hpp>
#include <clover2_camera_utils/calibration/target/chessboard.hpp>
#include <clover2_camera_utils/calibration/target/factory.hpp>

#include <array>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace clover2_camera_utils::calibration::target {
namespace {

using builder = std::shared_ptr<const base> (*)(const std::filesystem::path&);

const std::array<std::pair<std::string_view, builder>, 2> k_builders{
    std::pair{chessboard::name, &chessboard::from_file},
    std::pair{charuco::name, &charuco::from_file},
};

}  // namespace

std::shared_ptr<const base> factory::create(
    const std::string_view type, const std::filesystem::path& path) {
    for (const auto& [name, build] : k_builders) {
        if (type == name) {
            return build(path);
        }
    }

    std::ostringstream message;
    message << "unsupported board_type '" << type << "'; supported:";
    for (const auto& [name, build] : k_builders) {
        static_cast<void>(build);
        message << " " << name;
    }
    throw std::invalid_argument(message.str());
}

}  // namespace clover2_camera_utils::calibration::target
