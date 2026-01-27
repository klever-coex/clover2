#include <clover2/aruco/detector.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <lifecycle_msgs/msg/state.hpp>

#include <string>
#include <unordered_map>

namespace {

static constexpr const double transition_eps = 1e-4;
static constexpr const double rotation_eps = 1e-4;

const static std::unordered_map<std::string, int> marker_dictionary_map = {
    {"4X4_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50},
    {"4X4_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_100},
    {"4X4_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_250},
    {"4X4_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_1000},
    {"5X5_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_50},
    {"5X5_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_100},
    {"5X5_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_250},
    {"5X5_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_5X5_1000},
    {"6X6_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_50},
    {"6X6_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_100},
    {"6X6_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_250},
    {"6X6_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_6X6_1000},
    {"7X7_50", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_50},
    {"7X7_100", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_100},
    {"7X7_250", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_250},
    {"7X7_1000", cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_7X7_1000},
    {"ARUCO_ORIGINAL",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_ARUCO_ORIGINAL},
    {"APRILTAG_16h5",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_16h5},
    {"APRILTAG_25h9",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_25h9},
    {"APRILTAG_36h10",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_36h10},
    {"APRILTAG_36h11",
     cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_APRILTAG_36h11},
};

}  // namespace

namespace clover2::aruco {

detector::detector(const rclcpp::NodeOptions& options)
    : clover2::common::lifecycle_node("aruco_detector", options) {
    enable_watch_parameters();
    enable_diagnostic_updater();

    m_diagnostic_updater = get_diagnostic_updater();

    declare_and_watch_parameter<std::string>(
        "marker_dict", "4X4_250",
        [this](const rclcpp::Parameter& p) {
            auto dictionary_id = marker_dictionary_map.find(p.as_string());
            if (dictionary_id == marker_dictionary_map.end()) {
                throw std::runtime_error("invalid marker type " +
                                         p.as_string());
            }
            m_dictionary_name = p.as_string();
        },
        "Used marker dictionary");

    declare_and_watch_parameter<std::string>(
        "marker_frame_id", "aruco_",
        [this](const rclcpp::Parameter& p) {
            m_aruco_frame_id = p.as_string();
        },
        "Single marker frame_id prefix");

    declare_and_watch_parameter<bool>(
        "tf_publish", true,
        [this](const rclcpp::Parameter& p) { m_tf_publish = p.as_bool(); },
        "Enable map markers transform pub.");

    register_on_configure(
        std::bind(&detector::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&detector::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&detector::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&detector::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&detector::on_shutdown, this, std::placeholders::_1));
}

detector::CallbackReturn detector::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_detector_parameters = cv::aruco::DetectorParameters::create();
    m_dictionary =
        cv::makePtr<cv::aruco::Dictionary>(cv::aruco::getPredefinedDictionary(
            marker_dictionary_map.at(m_dictionary_name)));

    m_diagnostic_updater->add("Detector Status", this,
                              &detector::produce_diagnostics);

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(*this);

    m_map_client = std::make_shared<map_client>(shared_from_this());

    m_markers_pub =
        this->create_publisher<clover2_aruco_msgs::msg::MarkerArray>(
            "~/markers", rclcpp::SensorDataQoS());

    m_debug_pub = this->create_publisher<sensor_msgs::msg::Image>(
        "~/debug", rclcpp::SystemDefaultsQoS());

    m_camera_info_sub = this->create_subscription<sensor_msgs::msg::CameraInfo>(
        "~/camera_info", rclcpp::SensorDataQoS(),
        std::bind(&detector::camera_info_callback, this,
                  std::placeholders::_1));

    m_image_sub = this->create_subscription<sensor_msgs::msg::Image>(
        "~/image_raw", rclcpp::SensorDataQoS(),
        std::bind(&detector::image_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Activated.");

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster.reset();

    m_markers_pub.reset();
    m_debug_pub.reset();
    m_camera_info_sub.reset();
    m_image_sub.reset();

    m_map_client.reset();

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_detector_parameters.reset();
    m_dictionary.reset();

    m_diagnostic_updater->removeByName("Detector Status");

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster.reset();

    m_image_sub.reset();
    m_debug_pub.reset();
    m_camera_info_sub.reset();
    m_markers_pub.reset();

    m_dictionary.reset();
    m_detector_parameters.reset();

    m_map_client.reset();

    return detector::CallbackReturn::SUCCESS;
}

cv::Mat detector::marker_object_points(
    double length,
    const cv::Ptr<cv::aruco::EstimateParameters>& estimate_parameters) {
    cv::Mat objPoints(4, 1, CV_32FC3);

    if (estimate_parameters->pattern == cv::aruco::CW_top_left_corner) {
        objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(0.f, 0.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(length, 0.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(length, length, 0);
        objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(0.f, length, 0);
    } else if (estimate_parameters->pattern == cv::aruco::CCW_center) {
        objPoints.ptr<cv::Vec3f>(0)[0] =
            cv::Vec3f(-length / 2.f, length / 2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[1] =
            cv::Vec3f(length / 2.f, length / 2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[2] =
            cv::Vec3f(length / 2.f, -length / 2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[3] =
            cv::Vec3f(-length / 2.f, -length / 2.f, 0);
    } else {
        throw std::runtime_error("Invalid estimate pattern");
    }

    return objPoints;
}

const std::vector<cv::Point3f>& detector::get_marker_obj_points(
    int id, double length,
    const cv::Ptr<cv::aruco::EstimateParameters>& params) {
    auto it = m_marker_obj_cache.find(id);
    if (it != m_marker_obj_cache.end()) return it->second;

    std::vector<cv::Point3f> pts(4);

    if (params->pattern == cv::aruco::CW_top_left_corner) {
        pts = {{0, 0, 0},
               {float(length), 0, 0},
               {float(length), float(length), 0},
               {0, float(length), 0}};
    } else {
        float h = length * 0.5f;
        pts = {{-h, h, 0}, {h, h, 0}, {h, -h, 0}, {-h, -h, 0}};
    }

    return m_marker_obj_cache.emplace(id, pts).first->second;
}

void detector::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> camera_info_guard(m_camera_info_mtx);
    std::lock_guard<map_client> map_guard(*m_map_client);

    if (!m_map_client->valid()) {
        RCLCPP_ERROR(get_logger(), "Invalid map");
        return;
    }

    if (!m_camera_model.initialized()) {
        RCLCPP_ERROR(get_logger(), "Camera info not initialized");
        return;
    }

    cv::Mat image = cv_bridge::toCvShare(msg, "bgr8")->image;

    std::vector<int> ids;
    std::vector<cv::Point3f> obj_points;
    std::vector<std::vector<cv::Point2f>> corners, rejected;
    std::vector<geometry_msgs::msg::TransformStamped> transforms;

    std::unique_ptr<clover2_aruco_msgs::msg::MarkerArray> marker_array =
        std::make_unique<clover2_aruco_msgs::msg::MarkerArray>();
    marker_array->header = msg->header;

    cv::aruco::detectMarkers(image, m_dictionary, corners, ids,
                             m_detector_parameters, rejected);

    std::vector<bool> pose_estimated(ids.size(), false);
    std::vector<cv::Mat> marker_cov(ids.size());
    std::vector<cv::Vec3d> marker_rot(ids.size()), marker_pose(ids.size());

    if (ids.size() != 0) {
        auto estimate_parameters = cv::makePtr<cv::aruco::EstimateParameters>();

        parallel_for_(cv::Range(0, ids.size()), [&](const cv::Range& range) {
            const int begin = range.start;
            const int end = range.end;

            for (int i = begin; i < end; i++) {
                if (!m_map_client->has_marker(ids[i])) {
                    RCLCPP_WARN(get_logger(), "Marker %d not in map", ids[i]);
                    continue;
                }

                // cv::Mat marker_obj_points = marker_object_points(
                //     m_map_client->get_marker_size(ids[i]),
                //     estimate_parameters);

                const auto& obj_pts = get_marker_obj_points(
                    ids[i], m_map_client->get_marker_size(ids[i]),
                    estimate_parameters);

                std::vector<cv::Point2f> undist_corners;
                cv::fisheye::undistortPoints(
                    corners[i], undist_corners,
                    m_camera_model.fullIntrinsicMatrix(),
                    m_camera_model.distortionCoeffs(), cv::Mat(), m_k_rect);

                cv::solvePnP(obj_pts, undist_corners, m_k_rect, cv::Mat(),
                             marker_rot[i], marker_pose[i],
                             estimate_parameters->useExtrinsicGuess,
                             estimate_parameters->solvePnPMethod);

                // cv::solvePnP(obj_pts,                                 //
                //              cv::Mat(corners[i]),                     //
                //              m_camera_model.fullIntrinsicMatrix(),    //
                //              m_camera_model.distortionCoeffs(),       //
                //              marker_rot[i],                           //
                //              marker_pose[i],                          //
                //              estimate_parameters->useExtrinsicGuess,  //
                //              estimate_parameters->solvePnPMethod);

                compute_pose_covariance(obj_pts,         //
                                        marker_rot[i],   //
                                        marker_pose[i],  //
                                        m_k_rect,        //
                                        0.7,             //
                                        marker_cov[i]);

                pose_estimated[i] = true;
            }
        });

        for (size_t i = 0; i < ids.size(); i++) {
            if (!pose_estimated[i]) continue;

            // add marker
            clover2_aruco_msgs::msg::Marker marker;
            fill_corners(marker, corners[i]);
            fill_pose(marker, marker_rot[i], marker_pose[i]);
            fill_covariance(marker, marker_cov[i]);
            marker.id = ids[i];
            marker.size = m_map_client->get_marker_size(ids[i]);
            marker.marker_frame_id = m_map_client->get_marker_frame_id(ids[i]);
            marker_array->markers.push_back(marker);

            // add transform
            if (m_tf_publish) {
                geometry_msgs::msg::TransformStamped transform;
                transform.header = msg->header;
                transform.child_frame_id = get_marker_frame_id(ids[i]);
                transform.transform.rotation = marker.pose.pose.orientation;
                fill_translation(transform.transform.translation,
                                 marker_pose[i]);
                transforms.push_back(transform);
            }
        }
    }

    publish_detection(msg, std::move(marker_array), transforms, image, corners,
                      ids);
}

void detector::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    // validate camera info
    if (msg->height == 0 || msg->width == 0 || msg->d.size() == 0) {
        return;
    }

    m_camera_model.fromCameraInfo(msg);

    cv::Matx33d K = m_camera_model.fullIntrinsicMatrix();
    cv::Mat D = m_camera_model.distortionCoeffs();

    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::fisheye::estimateNewCameraMatrixForUndistortRectify(
        K, D, cv::Size(msg->width, msg->height), R, m_k_rect, 1.0);
}

void detector::compute_pose_covariance(const std::vector<cv::Point3f>& obj_pts,
                                       const cv::Vec3d& rvec,
                                       const cv::Vec3d& tvec, const cv::Mat& K,
                                       double sigma_pixel, cv::Mat& pose_cov) {
    const int N = obj_pts.size();
    cv::Mat J(2 * N, 6, CV_64F);
    std::vector<cv::Point2f> proj_p(N), proj_m(N);

    auto project = [&](const cv::Vec3d& r, const cv::Vec3d& t,
                       std::vector<cv::Point2f>& out) {
        cv::projectPoints(obj_pts, r, t, K, cv::Mat(), out);
    };

    // translation
    for (int i = 0; i < 3; i++) {
        cv::Vec3d dt(0, 0, 0);
        dt[i] = transition_eps;

        project(rvec, tvec + dt, proj_p);
        project(rvec, tvec - dt, proj_m);

        double inv = 1.0 / (2 * transition_eps);
        for (int k = 0; k < N; k++) {
            J.at<double>(2 * k, i) = (proj_p[k].x - proj_m[k].x) * inv;
            J.at<double>(2 * k + 1, i) = (proj_p[k].y - proj_m[k].y) * inv;
        }
    }

    // rotation
    for (int i = 0; i < 3; i++) {
        cv::Vec3d dr(0, 0, 0);
        dr[i] = rotation_eps;

        project(rvec + dr, tvec, proj_p);
        project(rvec - dr, tvec, proj_m);

        double inv = 1.0 / (2 * rotation_eps);
        for (int k = 0; k < N; k++) {
            J.at<double>(2 * k, i + 3) = (proj_p[k].x - proj_m[k].x) * inv;
            J.at<double>(2 * k + 1, i + 3) = (proj_p[k].y - proj_m[k].y) * inv;
        }
    }

    cv::Mat Sigma_img =
        cv::Mat::eye(2 * N, 2 * N, CV_64F) * sigma_pixel * sigma_pixel;
    pose_cov = (J.t() * Sigma_img.inv() * J).inv();
}

void detector::fill_corners(clover2_aruco_msgs::msg::Marker& marker,
                            const std::vector<cv::Point2f>& corners) const {
    marker.c1.x = corners[0].x;
    marker.c1.y = corners[0].y;

    marker.c2.x = corners[1].x;
    marker.c2.y = corners[1].y;

    marker.c3.x = corners[2].x;
    marker.c3.y = corners[2].y;

    marker.c4.x = corners[3].x;
    marker.c4.y = corners[3].y;
}

void detector::fill_pose(clover2_aruco_msgs::msg::Marker& marker,
                         const cv::Vec3d& rvec, const cv::Vec3d& tvec) const {
    marker.pose.pose.position.x = tvec[0];
    marker.pose.pose.position.y = tvec[1];
    marker.pose.pose.position.z = tvec[2];

    double angle = cv::norm(rvec);
    auto axis = rvec / angle;

    tf2::Quaternion q(tf2::Vector3(axis[0], axis[1], axis[2]), angle);
    marker.pose.pose.orientation.x = q.x();
    marker.pose.pose.orientation.y = q.y();
    marker.pose.pose.orientation.z = q.z();
    marker.pose.pose.orientation.w = q.w();
}

void detector::fill_covariance(clover2_aruco_msgs::msg::Marker& marker,
                               const cv::Mat& cov) {
    marker.pose.covariance.fill(0.0);

    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 6; c++) {
            marker.pose.covariance[r * 6 + c] = cov.at<double>(r, c);
        }
    }
}

void detector::fill_translation(geometry_msgs::msg::Vector3& translation,
                                const cv::Vec3d& tvec) const {
    translation.x = tvec[0];
    translation.y = tvec[1];
    translation.z = tvec[2];
}

void detector::publish_detection(
    const sensor_msgs::msg::Image::ConstSharedPtr& msg,
    std::unique_ptr<clover2_aruco_msgs::msg::MarkerArray> marker_array,
    const std::vector<geometry_msgs::msg::TransformStamped>& transforms,
    const cv::Mat& image, const std::vector<std::vector<cv::Point2f>>& corners,
    const std::vector<int>& ids) {
    // update for diagnostics
    m_last_marker_count = marker_array->markers.size();

    if (!transforms.empty() && m_tf_publish) {
        m_tf_broadcaster->sendTransform(transforms);
    }

    m_markers_pub->publish(std::move(marker_array));

    if (m_debug_pub->get_subscription_count() != 0) {
        cv::Mat debug = image.clone();
        cv::aruco::drawDetectedMarkers(debug, corners, ids);

        cv_bridge::CvImage cv_out;
        cv_out.header.frame_id = msg->header.frame_id;
        cv_out.header.stamp = msg->header.stamp;
        cv_out.encoding = sensor_msgs::image_encodings::BGR8;
        cv_out.image = debug;
        sensor_msgs::msg::Image::SharedPtr out_msg = cv_out.toImageMsg();
        m_debug_pub->publish(*out_msg);
    }
}

std::string detector::get_marker_frame_id(const int id) const {
    return m_aruco_frame_id + std::to_string(id);
}

void detector::produce_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_camera_model.initialized()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for Camera Info");
    } else if (!m_map_client || !m_map_client->valid()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Map Invalid or Missing");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "Running");
        stat.add("Map", m_map_client->get_name());
        stat.add("Camera Frame ID", m_camera_model.tfFrame());
        stat.add("Markers Detected", m_last_marker_count);
    }

    stat.add("Dictionary ID", m_dictionary_name);
}

}  // namespace clover2::aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::aruco::detector)
