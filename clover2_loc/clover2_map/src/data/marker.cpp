#include <clover2_map/data/marker.hpp>

// tf2
#include <tf2_eigen/tf2_eigen.hpp>

namespace clover2_map {

void marker::to_msg(clover2_pose_msgs::msg::Marker& msg) const {
    msg.id = static_cast<uint32_t>(id);
    msg.type = static_cast<uint8_t>(type);
    msg.size = static_cast<float>(size);
    msg.marker_frame_id = marker_frame_id;

    if (pose) {
        msg.pose.pose = tf2::toMsg(*pose);
    }
}

marker marker::from_msg(const clover2_pose_msgs::msg::Marker& msg) {
    marker mk;
    mk.id = static_cast<int>(msg.id);
    mk.type = clover2_map::marker_type(msg.type);
    mk.size = msg.size;
    mk.marker_frame_id = msg.marker_frame_id;

    if (mk.type == marker_type::fixed) {
        mk.pose = Eigen::Isometry3d::Identity();
        tf2::fromMsg(msg.pose.pose, *mk.pose);
    }

    return mk;
}

}  // namespace clover2_map
