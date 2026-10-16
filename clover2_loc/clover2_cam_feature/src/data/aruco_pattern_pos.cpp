#include <clover2/cam_feature/data/aruco_pattern_pos.hpp>

namespace clover2::cam_feature::data {

namespace {

const std::unordered_map<std::string, cv::aruco::PatternPos> str_to_pos = {
    {"center", cv::aruco::CCW_center},
    {"top_left", cv::aruco::CW_top_left_corner},
};

const std::unordered_map<cv::aruco::PatternPos, std::string> pos_to_str = {
    {cv::aruco::CCW_center, "center"},
    {cv::aruco::CW_top_left_corner, "top_left"},
};

}  // namespace

cv::aruco::PatternPos aruco_pattern_pos::from_str(const std::string& str) {
    auto it = str_to_pos.find(str);
    if (it != str_to_pos.end()) {
        return it->second;
    }

    throw std::invalid_argument("Invalid aruco_pattern_pos string.");
}

std::string aruco_pattern_pos::to_str(cv::aruco::PatternPos pos) {
    auto it = pos_to_str.find(pos);
    if (it != pos_to_str.end()) {
        return it->second;
    }

    throw std::invalid_argument("Invalid aruco_pattern_pos position.");
}

}  // namespace clover2::cam_feature::data
