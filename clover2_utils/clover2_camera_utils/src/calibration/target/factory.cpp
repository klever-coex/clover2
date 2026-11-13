#include <clover2_camera_utils/calibration/target/charuco.hpp>
#include <clover2_camera_utils/calibration/target/chessboard.hpp>
#include <clover2_camera_utils/calibration/target/factory.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace clover2_camera_utils::calibration::target {
factory::factory() {
    add<chessboard>();
    add<charuco>();
}

factory& factory::instance() {
    static factory instance;
    return instance;
}

bool factory::contains(const std::string_view type) const {
    return builders_.contains(std::string(type));
}

factory::value_type factory::create(const std::filesystem::path& path) const {
    if (path.empty() || !path.is_absolute()) {
        throw std::invalid_argument(
            "calibration target configuration path must be absolute");
    }

    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error(
            "could not open calibration target configuration '" +
            path.string() + "'");
    }

    nlohmann::json profile;
    try {
        profile = nlohmann::json::parse(input);
    } catch (const nlohmann::json::parse_error& error) {
        throw std::runtime_error(
            "could not parse calibration target configuration '" + path.string() +
            "': " + error.what());
    }

    const auto type = profile.at("type").get<std::string>();
    const auto iterator = builders_.find(type);
    if (iterator == builders_.end()) {
        std::ostringstream message;
        message << "unsupported calibration target type '" << type
                << "'; supported:";
        for (const auto& name : list_types()) {
            message << " " << name;
        }
        throw std::invalid_argument(message.str());
    }
    return iterator->second(profile);
}

std::vector<std::string> factory::list_types() const {
    std::vector<std::string> types;
    types.reserve(builders_.size());
    for (const auto& [type, build] : builders_) {
        static_cast<void>(build);
        types.push_back(type);
    }
    std::sort(types.begin(), types.end());
    return types;
}

}  // namespace clover2_camera_utils::calibration::target
