#include <clover2_camera_utils/calibration/detector/factory.hpp>

#include <clover2_camera_utils/calibration/detector/chessboard.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace clover2_camera_utils::calibration::detector {

factory::factory() {
    add<chessboard>();
}

factory& factory::instance() {
    static factory instance;
    return instance;
}

bool factory::contains(const std::string& board_type) const {
    return builders_.contains(board_type);
}

factory::value_type factory::create(
    const std::string& board_type) const {
    const auto iterator = builders_.find(board_type);
    if (iterator == builders_.end()) {
        std::ostringstream message;
        message << "unknown board type '" << board_type << "'; available:";
        for (const auto& name : list_board_types()) {
            message << " " << name;
        }
        throw std::invalid_argument(message.str());
    }
    return iterator->second();
}

std::vector<std::string> factory::list_board_types() const {
    std::vector<std::string> board_types;
    board_types.reserve(builders_.size());
    for (const auto& [board_type, builder] : builders_) {
        static_cast<void>(builder);
        board_types.push_back(board_type);
    }
    std::sort(board_types.begin(), board_types.end());
    return board_types;
}

}  // namespace clover2_camera_utils::calibration::detector
