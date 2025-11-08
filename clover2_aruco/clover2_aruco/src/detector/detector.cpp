#include <clover2_aruco/detector.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <lifecycle_msgs/msg/state.hpp>

#include <string>
#include <unordered_map>

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

namespace clover2_aruco {

detector::detector(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("aruco_detector", options) {
    enable_watch_parameters();

    declare_and_watch_parameter<std::string>(
        "marker_dict", "4X4_250",
        [this](const rclcpp::Parameter& p) {
            auto dictionary_id = marker_dictionary_map.find(p.as_string());
            if (dictionary_id == marker_dictionary_map.end()) {
                throw std::runtime_error("invalid marker type " +
                                         p.as_string());
            }
            m_dictionary_id = dictionary_id->second;
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
    m_dictionary = cv::makePtr<cv::aruco::Dictionary>(
        cv::aruco::getPredefinedDictionary(m_dictionary_id));

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

void detector::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> camera_info_guard(m_camera_info_mtx);
    std::lock_guard<map_client> map_guard(*m_map_client);

    if (!m_map_client->valid()) {
        RCLCPP_ERROR(get_logger(), "Invalid map");
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

                cv::Mat marker_obj_points = marker_object_points(
                    m_map_client->get_marker_size(ids[i]), estimate_parameters);

                cv::solvePnP(marker_obj_points, cv::Mat(corners[i]),
                             m_camera_model.fullIntrinsicMatrix(),
                             m_camera_model.distortionCoeffs(), marker_rot[i],
                             marker_pose[i],
                             estimate_parameters->useExtrinsicGuess,
                             estimate_parameters->solvePnPMethod);

                pose_estimated[i] = true;
            }
        });

        for (size_t i = 0; i < ids.size(); i++) {
            if (!pose_estimated[i]) continue;

            // add marker
            clover2_aruco_msgs::msg::Marker marker;
            fill_corners(marker, corners[i]);
            fill_pose(marker, marker_rot[i], marker_pose[i]);
            marker.id = ids[i];
            marker.size = m_map_client->get_marker_size(ids[i]);
            marker.marker_frame_id = m_map_client->get_marker_frame_id(ids[i]);
            marker_array->markers.push_back(marker);

            // add transform
            if (m_tf_publish) {
                geometry_msgs::msg::TransformStamped transform;
                transform.header = msg->header;
                transform.child_frame_id = get_marker_frame_id(ids[i]);
                transform.transform.rotation = marker.pose.orientation;
                fill_translation(transform.transform.translation,
                                 marker_pose[i]);
                transforms.push_back(transform);
            }
        }
    }

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

void detector::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    m_camera_model.fromCameraInfo(msg);
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
    marker.pose.position.x = tvec[0];
    marker.pose.position.y = tvec[1];
    marker.pose.position.z = tvec[2];

    double angle = cv::norm(rvec);
    auto axis = rvec / angle;

    tf2::Quaternion q(tf2::Vector3(axis[0], axis[1], axis[2]), angle);
    marker.pose.orientation.x = q.x();
    marker.pose.orientation.y = q.y();
    marker.pose.orientation.z = q.z();
    marker.pose.orientation.w = q.w();
}

void detector::fill_translation(geometry_msgs::msg::Vector3& translation,
                                const cv::Vec3d& tvec) const {
    translation.x = tvec[0];
    translation.y = tvec[1];
    translation.z = tvec[2];
}

std::string detector::get_marker_frame_id(const int id) const {
    return m_aruco_frame_id + std::to_string(id);
}

}  // namespace clover2_aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_aruco::detector)
