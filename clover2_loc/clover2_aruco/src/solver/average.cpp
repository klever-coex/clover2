// clover2
#include <clover2/aruco/solver/average.hpp>

// ROS2
#include <tf2_eigen/tf2_eigen.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

namespace clover2::aruco::solver {

average::average(base_solver::required_interfaces node_interfaces)
    : base_solver(std::move(node_interfaces)) {
    declare_and_watch_parameter<int>(
        "average.min_markers", 1,
        [this](const rclcpp::Parameter& p) {
            m_min_markers = static_cast<size_t>(p.as_int());
        },
        "Minimum fixed markers to publish a pose");
}

std::optional<Eigen::Isometry3d> average::solve(
    const clover2_pose_msgs::msg::MarkerFrame& frame,
    const clover2_map::map& map) {
    Eigen::Vector3d avg_translation = Eigen::Vector3d::Zero();
    Eigen::Vector4d cumulative_q = Eigen::Vector4d::Zero();
    size_t marker_count = 0;

    for (const auto& marker : frame.markers) {
        const auto it = map.markers.find(static_cast<int>(marker.id));
        if (it == map.markers.end()) {
            continue;
        }

        const auto& map_marker = it->second;
        if (map_marker.type != clover2_map::marker_type::fixed ||
            !map_marker.pose.has_value()) {
            continue;
        }

        Eigen::Isometry3d marker_pose = Eigen::Isometry3d::Identity();
        tf2::fromMsg(marker.pose.pose, marker_pose);

        const Eigen::Isometry3d camera_in_map =
            (*map_marker.pose) * marker_pose.inverse();

        avg_translation += camera_in_map.translation();
        const Eigen::Quaterniond q(camera_in_map.rotation());
        cumulative_q += q.coeffs();
        ++marker_count;
    }

    if (marker_count < m_min_markers) {
        return std::nullopt;
    }

    avg_translation /= static_cast<double>(marker_count);
    cumulative_q /= static_cast<double>(marker_count);
    Eigen::Quaterniond avg_quat = Eigen::Quaterniond::Identity();
    avg_quat.coeffs() = cumulative_q.normalized();

    Eigen::Isometry3d out = Eigen::Isometry3d::Identity();
    out.translate(avg_translation);
    out.rotate(avg_quat);

    return out;
}

}  // namespace clover2::aruco::solver
