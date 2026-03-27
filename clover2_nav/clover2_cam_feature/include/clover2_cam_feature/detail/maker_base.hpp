#pragma once

#include <clover2/aruco/map_client.hpp>
#include <clover2_cam_feature/base_plugin.hpp>

#include <opencv2/aruco.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace clover2_cam_feature::detail {

class maker_base : public clover2_cam_feature::base_plugin {
public:
    maker_base() = default;

    std::vector<geometry_msgs::msg::PoseWithCovarianceStamped> process(
        const cv::Mat& image, const cv::Matx33d& matrix,
        const cv::Mat_<double>& distortion,
        std::shared_ptr<cv::Mat> debug = nullptr) override;

protected:
    void bind_map_resources(const clover2_cam_feature::plugin_context& ctx) {
        m_map_client = ctx.map_client;
    }

    virtual void detect_markers(const cv::Mat& image, std::vector<int>& ids,
                                std::vector<std::vector<cv::Point2f>>& corners) = 0;

private:
    const std::vector<cv::Point3d>& get_marker_obj_points(
        int id, double length,
        const cv::Ptr<cv::aruco::EstimateParameters>& params);

    static void compute_pose_covariance(cv::Mat& pose_cov);

    static void fill_pose_stamped(
        geometry_msgs::msg::PoseWithCovarianceStamped& out,
        const cv::Vec3d& rvec, const cv::Vec3d& tvec, const cv::Mat& cov);

    std::shared_ptr<clover2::aruco::map_client> m_map_client;
    std::unordered_map<int, std::vector<cv::Point3d>> m_marker_obj_cache;
};

}  // namespace clover2_cam_feature::detail
