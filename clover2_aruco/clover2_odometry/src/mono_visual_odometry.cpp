#include <clover2_odometry/mono_visual_odometry.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

#include <cmath>
#include <fstream>
#include <sstream>

namespace clover2_odometry {

mono_visual_odometry::mono_visual_odometry(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("mono_visual_odometry", options)
    , m_R(cv::Mat::eye(3, 3, CV_64F))
    , m_t(cv::Mat::zeros(3, 1, CV_64F))
    , m_frame_id(0)
    , m_n_features(0)
    , m_lk_criteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 30, 0.01)
    , m_min_features(2000)
    , m_use_absolute_scale(false)
    , m_initialized(false)
    , m_odom_frame_id("odom")
    , m_base_frame_id("base_link") {
    enable_watch_parameters();

    // Declare parameters
    declare_and_watch_parameter<int>(
        "fast_threshold", 25,
        [this](const rclcpp::Parameter& p) {
            m_detector = cv::FastFeatureDetector::create(p.as_int(), true);
        },
        "FAST feature detector threshold");

    declare_and_watch_parameter<int>(
        "lk_win_size", 21,
        [this](const rclcpp::Parameter& p) {
            m_lk_win_size = p.as_int();
        },
        "Lucas-Kanade window size");

    declare_and_watch_parameter<int>(
        "min_features", 2000,
        [this](const rclcpp::Parameter& p) {
            m_min_features = static_cast<size_t>(p.as_int());
        },
        "Minimum number of features to track");

    declare_and_watch_parameter<bool>(
        "use_absolute_scale", false,
        [this](const rclcpp::Parameter& p) {
            m_use_absolute_scale = p.as_bool();
        },
        "Use absolute scale from ground truth poses");

    declare_and_watch_parameter<std::string>(
        "odom_frame_id", "odom",
        [this](const rclcpp::Parameter& p) { m_odom_frame_id = p.as_string(); },
        "Odometry frame ID");

    declare_and_watch_parameter<std::string>(
        "base_frame_id", "base_link",
        [this](const rclcpp::Parameter& p) { m_base_frame_id = p.as_string(); },
        "Base frame ID");

    // Initialize detector with default parameters
    m_detector = cv::FastFeatureDetector::create(25, true);

    register_on_configure(std::bind(&mono_visual_odometry::on_configure, this,
                                    std::placeholders::_1));
    register_on_activate(std::bind(&mono_visual_odometry::on_activate, this,
                                   std::placeholders::_1));
    register_on_deactivate(std::bind(&mono_visual_odometry::on_deactivate, this,
                                     std::placeholders::_1));
    register_on_cleanup(std::bind(&mono_visual_odometry::on_cleanup, this,
                                  std::placeholders::_1));
    register_on_shutdown(std::bind(&mono_visual_odometry::on_shutdown, this,
                                   std::placeholders::_1));
}

mono_visual_odometry::CallbackReturn mono_visual_odometry::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_diagnostic_updater = std::make_shared<diagnostic_updater::Updater>(this);
    m_diagnostic_updater->setHardwareID(this->get_name());
    m_diagnostic_updater->add("Visual Odometry Status", this,
                              &mono_visual_odometry::produce_diagnostics);

    return CallbackReturn::SUCCESS;
}

mono_visual_odometry::CallbackReturn mono_visual_odometry::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(*this);

    m_odom_pub = this->create_publisher<nav_msgs::msg::Odometry>(
        "~/odom", rclcpp::SystemDefaultsQoS());

    m_pose_pub = this->create_publisher<geometry_msgs::msg::PoseStamped>(
        "~/pose", rclcpp::SystemDefaultsQoS());

    m_pose_cov_pub =
        this->create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>(
            "~/pose_cov", rclcpp::SystemDefaultsQoS());

    m_debug_pub = this->create_publisher<sensor_msgs::msg::Image>(
        "~/debug", rclcpp::SystemDefaultsQoS());

    m_camera_info_sub = this->create_subscription<sensor_msgs::msg::CameraInfo>(
        "~/camera_info", rclcpp::SensorDataQoS(),
        std::bind(&mono_visual_odometry::camera_info_callback, this,
                  std::placeholders::_1));

    m_image_sub = this->create_subscription<sensor_msgs::msg::Image>(
        "~/image_raw", rclcpp::SensorDataQoS(),
        std::bind(&mono_visual_odometry::image_callback, this,
                  std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Activated.");

    return CallbackReturn::SUCCESS;
}

mono_visual_odometry::CallbackReturn mono_visual_odometry::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster.reset();

    m_odom_pub.reset();
    m_pose_pub.reset();
    m_pose_cov_pub.reset();
    m_debug_pub.reset();
    m_camera_info_sub.reset();
    m_image_sub.reset();

    return CallbackReturn::SUCCESS;
}

mono_visual_odometry::CallbackReturn mono_visual_odometry::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_diagnostic_updater.reset();
    m_detector.reset();

    return CallbackReturn::SUCCESS;
}

mono_visual_odometry::CallbackReturn mono_visual_odometry::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster.reset();

    m_image_sub.reset();
    m_debug_pub.reset();
    m_camera_info_sub.reset();
    m_odom_pub.reset();
    m_pose_pub.reset();
    m_pose_cov_pub.reset();

    m_detector.reset();
    m_diagnostic_updater.reset();

    return CallbackReturn::SUCCESS;
}

void mono_visual_odometry::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    // Validate camera info
    if (msg->height == 0 || msg->width == 0 || msg->d.size() == 0) {
        return;
    }

    m_camera_model.fromCameraInfo(msg);
}

void mono_visual_odometry::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> camera_info_guard(m_camera_info_mtx);

    if (!m_camera_model.initialized()) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "Camera info not initialized");
        return;
    }

    cv::Mat image = cv_bridge::toCvShare(msg, "mono8")->image;

    if (image.empty()) {
        RCLCPP_WARN(get_logger(), "Received empty image");
        return;
    }

    process_frame(image);

    // Publish odometry
    if (m_initialized) {
        cv::Vec3d adj_coord = get_mono_coordinates();

        // Create odometry message
        nav_msgs::msg::Odometry odom_msg;
        odom_msg.header.stamp = msg->header.stamp;
        odom_msg.header.frame_id = m_odom_frame_id;
        odom_msg.child_frame_id = m_base_frame_id;

        // Set pose
        odom_msg.pose.pose.position.x = adj_coord[0];
        odom_msg.pose.pose.position.y = adj_coord[1];
        odom_msg.pose.pose.position.z = adj_coord[2];

        // Convert rotation matrix to quaternion
        cv::Mat R_vec;
        cv::Rodrigues(m_R, R_vec);
        double angle = cv::norm(R_vec);
        if (angle > 1e-6) {
            cv::Vec3d axis_vec;
            R_vec.convertTo(axis_vec, CV_64F);
            axis_vec = axis_vec / angle;
            tf2::Quaternion q(
                tf2::Vector3(axis_vec[0], axis_vec[1], axis_vec[2]), angle);
            odom_msg.pose.pose.orientation.x = q.x();
            odom_msg.pose.pose.orientation.y = q.y();
            odom_msg.pose.pose.orientation.z = q.z();
            odom_msg.pose.pose.orientation.w = q.w();
        } else {
            odom_msg.pose.pose.orientation.w = 1.0;
        }

        // Set covariance (simple diagonal covariance)
        for (size_t i = 0; i < 36; ++i) {
            odom_msg.pose.covariance[i] = 0.0;
        }
        odom_msg.pose.covariance[0] = 0.1;    // x
        odom_msg.pose.covariance[7] = 0.1;    // y
        odom_msg.pose.covariance[14] = 0.1;   // z
        odom_msg.pose.covariance[21] = 0.05;  // roll
        odom_msg.pose.covariance[28] = 0.05;  // pitch
        odom_msg.pose.covariance[35] = 0.05;  // yaw

        m_odom_pub->publish(odom_msg);

        // Create pose message
        geometry_msgs::msg::PoseStamped pose_msg;
        pose_msg.header = odom_msg.header;
        pose_msg.pose = odom_msg.pose.pose;
        m_pose_pub->publish(pose_msg);

        // Create pose with covariance message
        geometry_msgs::msg::PoseWithCovarianceStamped pose_cov_msg;
        pose_cov_msg.header = odom_msg.header;
        pose_cov_msg.pose = odom_msg.pose;
        m_pose_cov_pub->publish(pose_cov_msg);

        // Publish transform
        geometry_msgs::msg::TransformStamped transform;
        transform.header = odom_msg.header;
        transform.child_frame_id = m_base_frame_id;
        transform.transform.translation.x = adj_coord[0];
        transform.transform.translation.y = adj_coord[1];
        transform.transform.translation.z = adj_coord[2];
        transform.transform.rotation = odom_msg.pose.pose.orientation;
        m_tf_broadcaster->sendTransform(transform);

        // Publish debug image if subscribers exist
        if (m_debug_pub->get_subscription_count() != 0) {
            cv::Mat debug = image.clone();
            if (debug.channels() == 1) {
                cv::cvtColor(debug, debug, cv::COLOR_GRAY2BGR);
            }

            // Draw tracked features
            for (size_t i = 0; i < m_good_new.size(); ++i) {
                cv::circle(debug, m_good_new[i], 3, cv::Scalar(0, 255, 0), -1);
                cv::line(debug, m_good_old[i], m_good_new[i],
                         cv::Scalar(0, 255, 0), 1);
            }

            cv_bridge::CvImage cv_out;
            cv_out.header.frame_id = msg->header.frame_id;
            cv_out.header.stamp = msg->header.stamp;
            cv_out.encoding = sensor_msgs::image_encodings::BGR8;
            cv_out.image = debug;
            sensor_msgs::msg::Image::SharedPtr out_msg = cv_out.toImageMsg();
            m_debug_pub->publish(*out_msg);
        }
    }
}

std::vector<cv::Point2f> mono_visual_odometry::detect_features(
    const cv::Mat& img) {
    std::vector<cv::KeyPoint> keypoints;
    m_detector->detect(img, keypoints);

    std::vector<cv::Point2f> points;
    points.reserve(keypoints.size());
    for (const auto& kp : keypoints) {
        points.push_back(kp.pt);
    }

    return points;
}

void mono_visual_odometry::visual_odometry() {
    // Redetect features if we have too few
    if (m_n_features < m_min_features || m_p0.empty()) {
        m_p0 = detect_features(m_old_frame);
        if (m_p0.empty()) {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
                                 "No features detected in frame");
            return;
        }
    }

    // Ensure we have valid points for optical flow
    if (m_p0.empty()) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
                             "No previous points for optical flow");
        return;
    }

    // Validate that frames are not empty
    if (m_old_frame.empty() || m_current_frame.empty()) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
                             "Empty frame detected");
        return;
    }

    // Validate that points are within image bounds
    cv::Size img_size = m_old_frame.size();
    std::vector<cv::Point2f> valid_p0;
    valid_p0.reserve(m_p0.size());
    for (const auto& pt : m_p0) {
        if (pt.x >= 0 && pt.x < img_size.width && pt.y >= 0 && pt.y < img_size.height) {
            valid_p0.push_back(pt);
        }
    }

    if (valid_p0.empty()) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
                             "No valid points within image bounds");
        m_p0.clear();
        m_n_features = 0;
        return;
    }

    m_p0 = valid_p0;

    // Calculate optical flow
    std::vector<uchar> status;
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(m_old_frame, m_current_frame, m_p0, m_p1, status,
                             err, cv::Size(m_lk_win_size, m_lk_win_size), 3, m_lk_criteria, 0, 0.001);

    // Filter good points
    m_good_old.clear();
    m_good_new.clear();
    for (size_t i = 0; i < status.size(); ++i) {
        if (status[i] == 1) {
            m_good_old.push_back(m_p0[i]);
            m_good_new.push_back(m_p1[i]);
        }
    }

    // Update p0 for next iteration even if we have too few features
    // This prevents stale data in m_p0
    if (!m_good_new.empty()) {
        m_p0 = m_good_new;
        m_n_features = m_good_new.size();
    } else {
        // If no good points, force redetection on next frame
        m_p0.clear();
        m_n_features = 0;
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
                             "No good features after optical flow");
        return;
    }

    if (m_good_new.size() < 8) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
                             "Too few features for odometry: %zu",
                             m_good_new.size());
        return;
    }

    // Find essential matrix
    cv::Mat mask;
    cv::Mat E = cv::findEssentialMat(m_good_new, m_good_old,
                                     m_camera_model.fullIntrinsicMatrix(),
                                     cv::RANSAC, 0.999, 1.0, mask);

    // Recover pose
    cv::Mat R, t;
    cv::recoverPose(E, m_good_old, m_good_new, m_camera_model.fullIntrinsicMatrix(), R, t, 50.0, mask);

    // Filter points using mask
    std::vector<cv::Point2f> filtered_old, filtered_new;
    for (int i = 0; i < mask.rows; ++i) {
        if (mask.at<uchar>(i) != 0) {
            filtered_old.push_back(m_good_old[i]);
            filtered_new.push_back(m_good_new[i]);
        }
    }
    m_good_old = filtered_old;
    m_good_new = filtered_new;

    // Initialize or update pose
    if (m_frame_id < 2) {
        R.convertTo(m_R, CV_64F);
        t.convertTo(m_t, CV_64F);
    } else {
        // Only update if translation is significant (mostly forward motion)
        if (abs(t.at<double>(2, 0)) > abs(t.at<double>(0, 0)) &&
            abs(t.at<double>(2, 0)) > abs(t.at<double>(1, 0))) {
            cv::Mat R_double, t_double;
            R.convertTo(R_double, CV_64F);
            t.convertTo(t_double, CV_64F);

            // Update cumulative pose
            cv::Mat t_world = m_R * t_double;
            m_t = m_t + t_world;
            m_R = R_double * m_R;
        }
    }

    // Feature count and p0 already updated above
}

cv::Vec3d mono_visual_odometry::get_mono_coordinates() const {
    // Apply coordinate system adjustment (diagonal matrix)
    cv::Mat diag = (cv::Mat_<double>(3, 3) << -1, 0, 0, 0, -1, 0, 0, 0, -1);
    cv::Mat adj_coord = diag * m_t;
    return cv::Vec3d(adj_coord.at<double>(0, 0), adj_coord.at<double>(1, 0),
                     adj_coord.at<double>(2, 0));
}

void mono_visual_odometry::process_frame(const cv::Mat& frame) {
    if (!m_initialized) {
        // Initialize with first frame
        m_old_frame = frame.clone();
        // Detect initial features
        m_p0 = detect_features(m_old_frame);
        m_n_features = m_p0.size();
        m_initialized = true;
        m_frame_id = 0;
        return;
    }

    // Set current frame
    m_current_frame = frame.clone();

    // Perform visual odometry
    visual_odometry();

    // Update for next iteration
    m_old_frame = m_current_frame.clone();
    m_frame_id++;
}

void mono_visual_odometry::produce_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_camera_model.initialized()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for Camera Info");
    } else if (!m_initialized) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Initializing");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "Running");
        stat.add("Features Tracked", static_cast<int>(m_n_features));
        stat.add("Frame ID", static_cast<int>(m_frame_id));
    }
}

}  // namespace clover2_odometry

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_odometry::mono_visual_odometry)
