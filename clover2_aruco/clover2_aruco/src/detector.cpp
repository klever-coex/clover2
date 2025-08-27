#include <clover2_aruco/detector.hpp>
#include <cv_bridge/cv_bridge.hpp>

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
    : rclcpp_lifecycle::LifecycleNode("aruco_detector", options)
    , m_camera_matrix(3, 3, CV_64FC1)
    , m_marker_obj_points(4, 1, CV_32FC3)
    , m_distortion_coeffs(4, 1, CV_64FC1, cv::Scalar(0)) {
    m_set_parameters_handle_ptr = add_on_set_parameters_callback(std::bind(
        &detector::on_set_parameters_cb, this, std::placeholders::_1));

    // Declare parameters
    this->declare_parameter("marker_dict", "4X4_50");
    this->declare_parameter("marker_size", 0.15);

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
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    RCLCPP_INFO(this->get_logger(), "Configure...");

    m_detector_parameters = cv::aruco::DetectorParameters::create();
    m_dictionary = cv::makePtr<cv::aruco::Dictionary>(
        cv::aruco::getPredefinedDictionary(m_dictionary_id));

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_tf_buffer = std::make_shared<tf2_ros::Buffer>(get_clock());
    m_tf_listener = std::make_shared<tf2_ros::TransformListener>(*m_tf_buffer);
    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(*this);

    m_markers_pub =
        this->create_publisher<clover2_aruco_msgs::msg::MarkerArray>(
            "~/detected", 10);
    m_image_sub = this->create_subscription<sensor_msgs::msg::Image>(
        "~/image_raw",
        rclcpp::QoS(
            rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_sensor_data)),
        std::bind(&detector::image_callback, this, std::placeholders::_1));
    m_camera_info_sub = this->create_subscription<sensor_msgs::msg::CameraInfo>(
        "~/camera_info",
        rclcpp::QoS(
            rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_sensor_data)),
        std::bind(&detector::camera_info_callback, this,
                  std::placeholders::_1));

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_tf_buffer.reset();
    m_tf_listener.reset();
    m_tf_broadcaster.reset();

    m_image_sub.reset();
    m_camera_info_sub.reset();
    m_markers_pub.reset();

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_dictionary.reset();
    m_detector_parameters.reset();

    return detector::CallbackReturn::SUCCESS;
}

detector::CallbackReturn detector::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_tf_buffer.reset();
    m_tf_listener.reset();
    m_tf_broadcaster.reset();

    m_image_sub.reset();
    m_camera_info_sub.reset();
    m_markers_pub.reset();

    m_dictionary.reset();
    m_detector_parameters.reset();

    return detector::CallbackReturn::SUCCESS;
}

void detector::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    cv::Mat image = cv_bridge::toCvShare(msg)->image;

    vector<int> ids;
    vector<vector<cv::Point2f>> corners, rejected;
    vector<cv::Vec3d> rvecs, tvecs;
    vector<cv::Point3f> obj_points;

    geometry_msgs::TransformStamped vertical;
    clover2_aruco_msgs::msg::MarkerArray marker_array;

    cv::aruco::detectMarkers(image, m_dictionary, corners, ids,
                             m_detector_parameters, rejected);

    marker_array.header.stamp = msg->header.stamp;
    marker_array.header.frame_id = msg->header.frame_id;

    if (ids.size() != 0) {
        cv::aruco::estimatePoseSingleMarkers(corners, m_marker_size,
                                             m_camera_matrix,
                                             m_distortion_coeffs, rvecs, tvecs);
    }
}

void detector::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    for (int i = 0; i < 9; ++i) {
        m_camera_matrix.at<double>(i / 3, i % 3) = msg->k[i];
    }

    m_distortion_coeffs = cv::Mat(msg->d, true);
}

detector::SetParametersResult detector::on_set_parameters_cb(
    const std::vector<rclcpp::Parameter>& parameters) {
    detector::SetParametersResult result;
    result.successful = true;

    for (auto& p : parameters) {
        try {
            if (p.get_name() == "marker_dict") {
                auto dictionary_id = marker_dictionary_map.find(p.as_string());
                if (dictionary_id == marker_dictionary_map.end()) {
                    std::runtime_error("invalid marker type " + p.as_string());
                }
                m_dictionary_id = dictionary_id->second;
            } else if (p.get_name() == "marker_dict") {
                m_marker_size = p.as_double();
            } else {
                std::runtime_error("unknown parameter name " + p.get_name());
            }
        } catch (std::exception& ex) {
            result.successful = false;
            result.reason = ex.what();
            RCLCPP_ERROR(get_logger(), "Fail set parameter `%s` with: %s",
                         p.get_name().c_str(), ex.what());
            break;
        }
    }

    return result;
}

}  // namespace clover2_aruco
