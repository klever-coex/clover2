#pragma once

// clover2
#include "geometry_msgs/msg/pose_array.hpp"
#include "std_msgs/msg/header.hpp"
#include <clover2/cam_feature/base_plugin.hpp>

// OpenCV
#include <clover2/common/node_context.hpp>
#include <opencv2/aruco.hpp>

// ROS2
#include <clover2_pose_msgs/msg/marker.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_with_covariance.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/subscription.hpp>

// STL
#include <memory>
#include <unordered_map>
#include <vector>

namespace clover2::cam_feature::detail {

class maker_base : public clover2::cam_feature::base_plugin {
public:
    explicit maker_base();
    virtual ~maker_base() = default;

    std::list<clover2_pose_msgs::msg::Marker> process(
        const std_msgs::msg::Header& header, const cv::Mat& image,
        const cv::Matx33d& matrix, const cv::Mat_<double>& distortion,
        std::shared_ptr<cv::Mat> debug = nullptr) override final;

protected:
    void _configure(const std::string& name,
                    const rclcpp_lifecycle::LifecycleNode::WeakPtr& node,
                    const std::shared_ptr<clover2::map::client>& map_client);
    void _activate();
    void _deactivate();
    void _cleanup();

    virtual void detect_markers(
        const cv::Mat& image, std::vector<int>& ids,
        std::vector<std::vector<cv::Point2f>>& corners) = 0;

private:
    const std::vector<cv::Point3d>& get_marker_obj_points(
        int id, double length,
        const cv::Ptr<cv::aruco::EstimateParameters>& params);

    static void compute_pose_covariance(cv::Mat& pose_cov);

    static void fill_pose_stamped(geometry_msgs::msg::PoseWithCovariance& out,
                                  const cv::Vec3d& rvec, const cv::Vec3d& tvec,
                                  const cv::Mat& cov);

    std::shared_ptr<clover2::map::client> m_map_client;
    rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr
        m_pose_array_debug_pub;

    std::unordered_map<int, std::vector<cv::Point3d>> m_marker_obj_cache;
};

}  // namespace clover2::cam_feature::detail
