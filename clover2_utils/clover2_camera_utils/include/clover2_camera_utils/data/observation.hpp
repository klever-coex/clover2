#pragma once

#include <opencv2/core/types.hpp>

#include <vector>

namespace clover2_camera_utils::data {

struct observation {
    cv::Size image_size;
    std::vector<cv::Point2f> image_points;
    std::vector<cv::Point3f> object_points;
};

}  // namespace clover2_camera_utils::data
