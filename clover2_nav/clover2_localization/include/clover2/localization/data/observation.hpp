#pragma once

#include <Eigen/Geometry>

#include <cstdint>

namespace clover2::localization::data {

struct observation {
    enum class type { marker_pose, keypoint_2d, point_3d };

    type obs_type = type::marker_pose;
    int32_t id = 0;

    Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
    Eigen::Vector2d pixel = Eigen::Vector2d::Zero();
    Eigen::Vector3d point3d = Eigen::Vector3d::Zero();

    Eigen::Isometry3d landmark_world = Eigen::Isometry3d::Identity();

    Eigen::MatrixXd information;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}  // namespace clover2::localization::data
