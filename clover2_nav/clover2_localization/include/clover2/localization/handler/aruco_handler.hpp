#pragma once

#include <clover2/localization/data/frame_data.hpp>
#include <clover2/localization/data/observation.hpp>
#include <clover2/localization/data/sensor_data.hpp>
#include <clover2/localization/handler/handler_interface.hpp>

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace clover2::localization::handler {

class aruco_handler : public base_handler {
public:
    struct config {
        std::string dictionary = "DICT_4X4_50";
        double default_marker_size = 0.3;
        std::unordered_map<int, double> marker_sizes;
        std::unordered_map<int, Eigen::Isometry3d> marker_poses;
    };

    explicit aruco_handler(config cfg);

    data::frame_data process(const data::sensor_data& data) override;

private:
    cv::Ptr<cv::aruco::Dictionary> get_dictionary(const std::string& name);
    Eigen::Isometry3d rvec_tvec_to_isometry(const cv::Vec3d& rvec,
                                            const cv::Vec3d& tvec) const;

    config m_config;
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
};

}  // namespace clover2::localization::handler
