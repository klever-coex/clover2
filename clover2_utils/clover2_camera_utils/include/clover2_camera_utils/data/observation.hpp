/**
 * @file observation.hpp
 * @brief Provides a calibration board observation.
 */

#pragma once

#include <opencv2/core/types.hpp>

#include <vector>

namespace clover2_camera_utils::data {

/** @brief Stores calibration board points detected in one image. */
struct observation {
    cv::Size image_size;                    ///< Source image size.
    std::vector<cv::Point2f> image_points;  ///< Corner coordinates in the image.
    std::vector<cv::Point3f>
        object_points;  ///< Corner coordinates in the board frame.
};

}  // namespace clover2_camera_utils::data
