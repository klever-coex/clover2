#pragma once

// opencv
#include <opencv2/calib3d.hpp>

// STL
#include <string>

namespace clover2::cam_feature::data {

struct solve_pnp_method {
    static cv::SolvePnPMethod from_str(const std::string& str);
    static std::string to_str(cv::SolvePnPMethod method);
};

}  // namespace clover2::cam_feature::data
