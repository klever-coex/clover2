#pragma once

// clover2
#include <clover2_map/data/marker_type.hpp>
#include <clover2_pose_msgs/msg/marker.hpp>

// Eigen
#include <Eigen/Geometry>

// STL
#include <optional>
#include <string>

namespace clover2_map {

struct marker {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    int id = 0;
    marker_type type;
    double size = -1.0;
    std::optional<Eigen::Isometry3d> pose;
    std::string marker_frame_id;

    void to_msg(clover2_pose_msgs::msg::Marker& msg) const;
    static marker from_msg(const clover2_pose_msgs::msg::Marker& msg);
};

}  // namespace clover2_map
