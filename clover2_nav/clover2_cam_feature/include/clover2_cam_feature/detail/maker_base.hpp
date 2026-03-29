#pragma once

// clover2
#include <clover2_cam_feature/base_plugin.hpp>
#include <clover2_localization_msgs/helpers.hpp>

// OpenCV
#include <opencv2/aruco.hpp>

// msgs
#include <clover2_localization_msgs/msg/feature3.hpp>
#include <geometry_msgs/msg/pose_with_covariance.hpp>

// STL
#include <memory>
#include <unordered_map>
#include <vector>

namespace clover2_cam_feature::detail {

class maker_base : public clover2_cam_feature::base_plugin {
public:
    maker_base() = default;

    std::list<clover2_localization_msgs::msg::Feature3> process(
        const cv::Mat& image, const cv::Matx33d& matrix,
        const cv::Mat_<double>& distortion,
        std::shared_ptr<cv::Mat> debug = nullptr) override;

protected:
    virtual void detect_markers(const cv::Mat& image, std::vector<int>& ids,
                                std::vector<std::vector<cv::Point2f>>& corners) = 0;

    virtual clover2_localization_msgs::helpers::feature_type feature_type() const;

private:
    const std::vector<cv::Point3d>& get_marker_obj_points(
        int id, double length,
        const cv::Ptr<cv::aruco::EstimateParameters>& params);

    static void compute_pose_covariance(cv::Mat& pose_cov);

    static void fill_pose_stamped(
        geometry_msgs::msg::PoseWithCovariance& out,
        const cv::Vec3d& rvec, const cv::Vec3d& tvec, const cv::Mat& cov);

    std::unordered_map<int, std::vector<cv::Point3d>> m_marker_obj_cache;
};

}  // namespace clover2_cam_feature::detail
