#pragma once

#include <clover2/localization/data/observation.hpp>

#include <Eigen/Geometry>

#include <vector>

namespace clover2::localization::data {

struct frame_data {
    double timestamp = 0.0;
    int32_t sensor_id = 0;
    std::vector<observation, Eigen::aligned_allocator<observation>> observations;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}  // namespace clover2::localization::data
