#include <clover2_pose_msgs/marker.hpp>

// ROS2
#include <tf2_eigen/tf2_eigen.hpp>

namespace clover2_pose_msgs {

marker::marker(const clover2_pose_msgs::msg::Marker& msg)
    : id(msg.id)
    , size(msg.size)
    , marker_frame_id(msg.marker_frame_id) {
    tf2::fromMsg(msg.pose.pose, transform);
}

marker::marker(const marker& other)
    : id(other.id)
    , size(other.size)
    , transform(other.transform)
    , marker_frame_id(other.marker_frame_id) {}

marker& marker::operator=(const marker& other) {
    if (this != &other) {
        id = other.id;
        size = other.size;
        transform = other.transform;
        marker_frame_id = other.marker_frame_id;
    }

    return *this;
}

}  // namespace clover2_pose_msgs
