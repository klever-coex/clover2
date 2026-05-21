#pragma once

// Eigen
#include <Eigen/Geometry>

// ROS
#include <clover2_pose_msgs/msg/marker.hpp>

namespace clover2_pose_msgs {

class marker {
public:
    marker() = default;
    marker(const clover2_pose_msgs::msg::Marker& msg);

    marker(const marker& other);
    marker& operator=(const marker& other);

    int id;
    double size;
    Eigen::Isometry3d transform;
    std::string marker_frame_id;
};

}  // namespace clover2_pose_msgs
