#include <clover2/pose/handler/aruco_handler.hpp>

#include <opencv2/calib3d.hpp>

#include <Eigen/Geometry>

#include <stdexcept>

namespace clover2::pose::handler {

namespace {

const std::unordered_map<std::string, int> k_dict_map = {
    {"DICT_4X4_50", cv::aruco::DICT_4X4_50},
    {"DICT_4X4_100", cv::aruco::DICT_4X4_100},
    {"DICT_4X4_250", cv::aruco::DICT_4X4_250},
    {"DICT_4X4_1000", cv::aruco::DICT_4X4_1000},
    {"DICT_5X5_50", cv::aruco::DICT_5X5_50},
    {"DICT_5X5_100", cv::aruco::DICT_5X5_100},
    {"DICT_5X5_250", cv::aruco::DICT_5X5_250},
    {"DICT_5X5_1000", cv::aruco::DICT_5X5_1000},
    {"DICT_6X6_50", cv::aruco::DICT_6X6_50},
    {"DICT_6X6_250", cv::aruco::DICT_6X6_250},
    {"DICT_6X6_1000", cv::aruco::DICT_6X6_1000},
    {"DICT_7X7_50", cv::aruco::DICT_7X7_50},
    {"DICT_7X7_250", cv::aruco::DICT_7X7_250},
    {"DICT_7X7_1000", cv::aruco::DICT_7X7_1000},
};

}  // namespace

aruco_handler::aruco_handler(config cfg)
    : m_config(std::move(cfg)),
      m_dictionary(get_dictionary(m_config.dictionary)) {}

cv::Ptr<cv::aruco::Dictionary> aruco_handler::get_dictionary(
    const std::string& name) {
    auto it = k_dict_map.find(name);
    if (it == k_dict_map.end()) {
        throw std::runtime_error("Unknown ArUco dictionary: " + name);
    }
    return cv::aruco::getPredefinedDictionary(it->second);
}

Eigen::Isometry3d aruco_handler::rvec_tvec_to_isometry(const cv::Vec3d& rvec,
                                                       const cv::Vec3d& tvec) const {
    cv::Mat R;
    cv::Rodrigues(rvec, R);

    Eigen::Isometry3d T = Eigen::Isometry3d::Identity();
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            T.linear()(r, c) = R.at<double>(r, c);
        }
    }
    T.translation() =
        Eigen::Vector3d(tvec[0], tvec[1], tvec[2]);
    return T;
}

data::frame_data aruco_handler::process(const data::sensor_data& data) {
    data::frame_data frame;
    frame.timestamp = data.timestamp;
    frame.sensor_id = data.sensor_id;

    if (!data.image.has_value() || !data.intrinsics.has_value()) {
        return frame;
    }

    const cv::Mat& image = data.image.value();
    const auto& intrinsics = data.intrinsics.value();

    if (image.empty() || intrinsics.K.empty()) {
        return frame;
    }

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;

    cv::aruco::detectMarkers(image, m_dictionary, corners, ids);

    if (ids.empty()) {
        return frame;
    }

    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::aruco::estimatePoseSingleMarkers(corners, m_config.default_marker_size,
                                         intrinsics.K, intrinsics.dist_coeffs,
                                         rvecs, tvecs);

    for (size_t i = 0; i < ids.size(); ++i) {
        int marker_id = ids[i];
        auto it = m_config.marker_poses.find(marker_id);
        if (it == m_config.marker_poses.end()) {
            continue;
        }

        data::observation obs;
        obs.obs_type = data::observation::type::marker_pose;
        obs.id = marker_id;
        obs.pose = rvec_tvec_to_isometry(rvecs[i], tvecs[i]);
        obs.landmark_world = it->second;
        obs.information = Eigen::Matrix<double, 6, 6>::Identity();

        frame.observations.push_back(std::move(obs));
    }

    return frame;
}

}  // namespace clover2::pose::handler
