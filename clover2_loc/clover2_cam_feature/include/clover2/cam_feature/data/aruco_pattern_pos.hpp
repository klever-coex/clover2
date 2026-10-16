#pragma once

// opencv
#include <opencv2/aruco.hpp>

// STL
#include <string>

namespace clover2::cam_feature::data {

struct aruco_pattern_pos {
    static cv::aruco::PatternPos from_str(const std::string& str);
    static std::string to_str(cv::aruco::PatternPos pos);
};

}  // namespace clover2::cam_feature::data
