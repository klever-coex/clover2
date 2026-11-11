#pragma once

// clover2
#include <clover2_fcu/common/frame.hpp>

// eigen
#include <Eigen/Geometry>

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_fcu::data {

struct odometry {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Eigen::Vector3d position{Eigen::Vector3d::Zero()};
    Eigen::Vector3d velocity{Eigen::Vector3d::Zero()};
    Eigen::Quaterniond attitude{Eigen::Quaterniond::Identity()};
    Eigen::Vector3d angular_velocity{Eigen::Vector3d::Zero()};

    rclcpp::Time stamp{};
};

}  // namespace clover2_fcu::data
