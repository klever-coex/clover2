#include <clover2_cam_feature/detail/maker_base.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core/utility.hpp>
#include <tf2/LinearMath/Quaternion.h>

namespace clover2_cam_feature::detail {

const std::vector<cv::Point3d>& maker_base::get_marker_obj_points(
    int id, double length,
    const cv::Ptr<cv::aruco::EstimateParameters>& params) {
    auto it = m_marker_obj_cache.find(id);
    if (it != m_marker_obj_cache.end()) {
        return it->second;
    }

    std::vector<cv::Point3d> pts(4);

    if (params->pattern == cv::aruco::CW_top_left_corner) {
        pts[0] = cv::Vec3d(0.f, 0.f, 0);
        pts[1] = cv::Vec3d(length, 0.f, 0);
        pts[2] = cv::Vec3d(length, length, 0);
        pts[3] = cv::Vec3d(0.f, length, 0);
    } else {
        double h = length * 0.5f;
        pts[0] = cv::Vec3d(-h, h, 0);
        pts[1] = cv::Vec3d(h, h, 0);
        pts[2] = cv::Vec3d(h, -h, 0);
        pts[3] = cv::Vec3d(-h, -h, 0);
    }

    return m_marker_obj_cache.emplace(id, std::move(pts)).first->second;
}

void maker_base::compute_pose_covariance(cv::Mat& pose_cov) {
    pose_cov.create(6, 6, CV_64F);
    pose_cov.setTo(0);

    pose_cov.at<double>(0, 0) = 0.05f;
    pose_cov.at<double>(1, 1) = 0.05f;
    pose_cov.at<double>(2, 2) = 0.1f;

    pose_cov.at<double>(3, 3) = 0.01f;
    pose_cov.at<double>(4, 4) = 0.01f;
    pose_cov.at<double>(5, 5) = 0.05f;
}

void maker_base::fill_pose_stamped(
    geometry_msgs::msg::PoseWithCovarianceStamped& out, const cv::Vec3d& rvec,
    const cv::Vec3d& tvec, const cv::Mat& cov) {
    out.pose.pose.position.x = tvec[0];
    out.pose.pose.position.y = tvec[1];
    out.pose.pose.position.z = tvec[2];

    double angle = cv::norm(rvec);
    auto axis = rvec / angle;

    tf2::Quaternion q(tf2::Vector3(axis[0], axis[1], axis[2]), angle);
    out.pose.pose.orientation.x = q.x();
    out.pose.pose.orientation.y = q.y();
    out.pose.pose.orientation.z = q.z();
    out.pose.pose.orientation.w = q.w();

    out.pose.covariance.fill(0.0);
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 6; c++) {
            out.pose.covariance[r * 6 + c] = cov.at<double>(r, c);
        }
    }
}

std::vector<geometry_msgs::msg::PoseWithCovarianceStamped> maker_base::process(
    const cv::Mat& image, const cv::Matx33d& matrix,
    const cv::Mat_<double>& distortion, std::shared_ptr<cv::Mat> debug) {
    std::vector<geometry_msgs::msg::PoseWithCovarianceStamped> result;

    if (!m_map_client) {
        RCLCPP_ERROR(get_logger(), "map_client is null");
        return result;
    }

    std::lock_guard<clover2::aruco::map_client> map_guard(*m_map_client);

    if (!m_map_client->valid()) {
        RCLCPP_ERROR(get_logger(), "Invalid map");
        return result;
    }

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    detect_markers(image, ids, corners);

    const cv::Mat camera_matrix(matrix);
    const cv::Mat dist_coeffs(distortion);

    std::vector<bool> pose_estimated(ids.size(), false);
    std::vector<cv::Mat> marker_cov(ids.size());
    std::vector<cv::Vec3d> marker_rot(ids.size()), marker_pose(ids.size());

    result.reserve(ids.size());

    if (!ids.empty()) {
        auto estimate_parameters = cv::makePtr<cv::aruco::EstimateParameters>();

        parallel_for_(cv::Range(0, static_cast<int>(ids.size())),
                      [&](const cv::Range& range) {
                          for (int i = range.start; i < range.end; i++) {
                              if (!m_map_client->has_marker(ids[i])) {
                                  continue;
                              }

                              const auto& obj_pts = get_marker_obj_points(
                                  ids[i], m_map_client->get_marker_size(ids[i]),
                                  estimate_parameters);

                              cv::solvePnP(
                                  obj_pts, cv::Mat(corners[i]), camera_matrix,
                                  dist_coeffs, marker_rot[i], marker_pose[i],
                                  estimate_parameters->useExtrinsicGuess,
                                  estimate_parameters->solvePnPMethod);

                              compute_pose_covariance(marker_cov[i]);

                              pose_estimated[i] = true;
                          }
                      });

        for (size_t i = 0; i < ids.size(); i++) {
            if (!pose_estimated[i]) {
                continue;
            }

            geometry_msgs::msg::PoseWithCovarianceStamped pose_stamped;
            pose_stamped.header.stamp = get_clock()->now();
            fill_pose_stamped(pose_stamped, marker_rot[i], marker_pose[i],
                              marker_cov[i]);
            result.push_back(std::move(pose_stamped));
        }
    }

    if (debug) {
        cv::aruco::drawDetectedMarkers(*debug, corners, ids);
    }

    return result;
}

}  // namespace clover2_cam_feature::detail
