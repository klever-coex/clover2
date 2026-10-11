// clover2
#include <clover2/aruco/solver/global_pnp.hpp>

// third party
#include <opencv2/calib3d.hpp>
#include <opencv2/core/eigen.hpp>

// STL
#include <array>

namespace clover2::aruco::solver {

global_pnp::global_pnp(base_solver::required_interfaces node_interfaces)
    : base_solver(std::move(node_interfaces)) {
    declare_and_watch_parameter<int>(
        "global_pnp.ransac_iterations", 100,
        [this](const rclcpp::Parameter& p) {
            m_ransac_iterations = p.as_int();
        },
        "RANSAC iteration count for the global PnP");

    declare_and_watch_parameter<double>(
        "global_pnp.ransac_reproj_px", 2.5,
        [this](const rclcpp::Parameter& p) {
            m_ransac_reproj_px = p.as_double();
        },
        "RANSAC inlier reprojection threshold[px]");

    declare_and_watch_parameter<int>(
        "global_pnp.min_markers", 1,
        [this](const rclcpp::Parameter& p) {
            m_min_markers = static_cast<size_t>(p.as_int());
        },
        "Minimum fixed markers with corners to publish a pose");
}

std::optional<Eigen::Isometry3d> global_pnp::solve(
    const clover2_pose_msgs::msg::MarkerFrame& frame,
    const clover2_map::map& map) {
    if (frame.camera_info.width == 0 || frame.camera_info.height == 0) {
        return std::nullopt;
    }

    m_camera_model.fromCameraInfo(frame.camera_info);

    const auto camera_matrix = m_camera_model.fullIntrinsicMatrix();
    const auto distortion = m_camera_model.distortionCoeffs();

    size_t marker_count = 0;
    std::vector<cv::Point3f> object_points;
    std::vector<cv::Point2f> image_points;

    for (const auto& marker : frame.markers) {
        const auto it = map.markers.find(static_cast<int>(marker.id));
        if (it == map.markers.end()) {
            continue;
        }

        const auto& map_marker = it->second;
        if (map_marker.type != clover2_map::marker_type::fixed ||
            !map_marker.pose.has_value() || marker.corner_px.size() != 4) {
            continue;
        }

        ++marker_count;

        const double h = map_marker.size / 2.0;
        const std::array<cv::Point3d, 4> local = {
            cv::Point3d(-h, h, 0),  //
            cv::Point3d(h, h, 0),   //
            cv::Point3d(h, -h, 0),  //
            cv::Point3d(-h, -h, 0)  //
        };

        for (size_t j = 0; j < local.size(); ++j) {
            const Eigen::Vector3d corner =
                (*map_marker.pose) *
                Eigen::Vector3d(local[j].x, local[j].y, local[j].z);

            object_points.emplace_back(static_cast<float>(corner.x()),
                                       static_cast<float>(corner.y()),
                                       static_cast<float>(corner.z()));

            image_points.emplace_back(
                static_cast<float>(marker.corner_px[j].x),
                static_cast<float>(marker.corner_px[j].y));
        }
    }

    if (marker_count < m_min_markers) {
        return std::nullopt;
    }

    cv::Mat rvec, tvec;
    if (marker_count == 1) {
        std::vector<cv::Mat> rvecs, tvecs;
        cv::Mat reproj_errors;

        if (cv::solvePnPGeneric(object_points, image_points, camera_matrix,
                                distortion, rvecs, tvecs, false,
                                cv::SOLVEPNP_IPPE, cv::noArray(), cv::noArray(),
                                reproj_errors) == 0) {
            return std::nullopt;
        }

        rvec = rvecs.front();
        tvec = tvecs.front();
    } else if (!cv::solvePnPRansac(object_points, image_points, camera_matrix,
                                   distortion, rvec, tvec, false,
                                   m_ransac_iterations,
                                   static_cast<float>(m_ransac_reproj_px), 0.99,
                                   cv::noArray(), cv::SOLVEPNP_SQPNP)) {
        return std::nullopt;
    }

    cv::Mat rotation_cv;
    cv::Rodrigues(rvec, rotation_cv);

    Eigen::Matrix3d rotation;
    cv::cv2eigen(rotation_cv, rotation);

    Eigen::Isometry3d camera_in_map = Eigen::Isometry3d::Identity();
    camera_in_map.linear() = rotation.inverse();
    camera_in_map.translation() =
        -rotation.inverse() * Eigen::Vector3d(tvec.at<double>(0),
                                              tvec.at<double>(1),
                                              tvec.at<double>(2));

    return camera_in_map;
}

}  // namespace clover2::aruco::solver
