
// clover2
#include <clover2/cam_feature/detail/maker_base.hpp>

// opencv
#include <opencv2/calib3d.hpp>
#include <opencv2/core/utility.hpp>

// tf2
#include <rclcpp/create_publisher.hpp>
#include <tf2/LinearMath/Quaternion.h>

namespace clover2::cam_feature::detail {

maker_base::maker_base()
    : m_map_client(nullptr)
    , m_pose_array_debug_pub(nullptr) {}

void maker_base::_configure(
    [[maybe_unused]] const std::string& name,
    [[maybe_unused]] const rclcpp_lifecycle::LifecycleNode::WeakPtr& node,
    const std::shared_ptr<clover2::map::client>& map_client) {
    m_map_client = map_client;
}

void maker_base::_activate() {
    m_pose_array_debug_pub =
        rclcpp::create_publisher<geometry_msgs::msg::PoseArray>(
            m_node_context, "~/output/" + get_name() + "/debug",
            rclcpp::QoS(10));
}

void maker_base::_deactivate() {  //
    m_pose_array_debug_pub.reset();
}

void maker_base::_cleanup() {  //
    m_map_client.reset();
}

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

void maker_base::compute_pose_covariance([[maybe_unused]] const cv::Vec3d& rvec,
                                         const cv::Vec3d& tvec,
                                         cv::Mat& pose_cov) {
    pose_cov.create(6, 6, CV_64F);
    pose_cov.setTo(0);

    auto distance = cv::norm(tvec);

    // TODO: https://www.youtube.com/shorts/PLbQuKxKaIs
    pose_cov.at<double>(0, 0) = 0.05f * distance;
    pose_cov.at<double>(1, 1) = 0.05f * distance;
    pose_cov.at<double>(2, 2) = 0.1f * distance;

    pose_cov.at<double>(3, 3) = 0.01f;
    pose_cov.at<double>(4, 4) = 0.01f;
    pose_cov.at<double>(5, 5) = 0.05f;
}

void maker_base::fill_pose_stamped(geometry_msgs::msg::PoseWithCovariance& out,
                                   const cv::Vec3d& rvec, const cv::Vec3d& tvec,
                                   const cv::Mat& cov) {
    out.pose.position.x = tvec[0];
    out.pose.position.y = tvec[1];
    out.pose.position.z = tvec[2];

    double angle = cv::norm(rvec);
    auto axis = rvec / angle;

    tf2::Quaternion q(tf2::Vector3(axis[0], axis[1], axis[2]), angle);
    out.pose.orientation.x = q.x();
    out.pose.orientation.y = q.y();
    out.pose.orientation.z = q.z();
    out.pose.orientation.w = q.w();

    out.covariance.fill(0.0);
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 6; c++) {
            out.covariance[r * 6 + c] = cov.at<double>(r, c);
        }
    }
}

std::list<clover2_pose_msgs::msg::Marker> maker_base::process(
    const std_msgs::msg::Header& header, const cv::Mat& image,
    const cv::Matx33d& matrix, const cv::Mat_<double>& distortion,
    std::shared_ptr<cv::Mat> debug) {
    std::list<clover2_pose_msgs::msg::Marker> result;

    if (!m_map_client) {
        RCLCPP_ERROR(get_logger(), "map_client is null");
        return result;
    }

    if (!m_map_client->valid()) {
        RCLCPP_ERROR(get_logger(), "Invalid map");
        return result;
    }

    geometry_msgs::msg::PoseArray debug_msg;
    debug_msg.header = header;

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    detect_markers(image, ids, corners);

    std::vector<bool> pose_estimated(ids.size(), false);
    std::vector<cv::Mat> marker_cov(ids.size());
    std::vector<cv::Vec3d> marker_rot(ids.size()), marker_pose(ids.size());

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
                                  obj_pts, cv::Mat(corners[i]), matrix,
                                  distortion, marker_rot[i], marker_pose[i],
                                  estimate_parameters->useExtrinsicGuess,
                                  estimate_parameters->solvePnPMethod);

                              compute_pose_covariance(
                                  marker_rot[i], marker_pose[i], marker_cov[i]);

                              pose_estimated[i] = true;
                          }
                      });

        for (size_t i = 0; i < ids.size(); i++) {
            if (!pose_estimated[i]) {
                continue;
            }

            clover2_pose_msgs::msg::Marker marker;
            marker.id = ids[i];
            marker.size = m_map_client->get_marker_size(ids[i]);
            marker.marker_frame_id = m_map_client->get_marker_frame_id(ids[i]);
            fill_pose_stamped(marker.pose, marker_rot[i], marker_pose[i],
                              marker_cov[i]);
            result.push_back(std::move(marker));

            debug_msg.poses.push_back(marker.pose.pose);
        }
    }

    if (m_pose_array_debug_pub->get_subscription_count()) {
        m_pose_array_debug_pub->publish(debug_msg);
    }

    if (debug) {
        cv::aruco::drawDetectedMarkers(*debug, corners, ids);
    }

    return result;
}

}  // namespace clover2::cam_feature::detail
