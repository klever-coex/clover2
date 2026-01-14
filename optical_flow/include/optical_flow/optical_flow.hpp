#pragma once

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/publisher.hpp>

#include <image_geometry/pinhole_camera_model.hpp>

#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.hpp>

#include <geometry_msgs/msg/vector3_stamped.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <mavros_msgs/msg/optical_flow_rad.hpp>

#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

class OpticalFlow : public rclcpp::Node {
public:
    OpticalFlow(const rclcpp::NodeOptions& options);

private:
    void camera_info_callback(const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg);
    void flow_calc(const sensor_msgs::msg::Image::ConstSharedPtr& msg, const sensor_msgs::msg::CameraInfo::ConstSharedPtr& cinfo);
    void drawFlow(cv::Mat& frame, cv::Point2f shift, double quality);

    rclcpp::Time m_prev_stamp;
    
    int _roi_size;
    cv::Rect _roi;
    cv::Mat _hann;
	cv::Mat _prev, _curr;
	// cv::Mat _camera_matrix, _dist_vector;
    image_geometry::PinholeCameraModel m_camera_model;

    tf2_ros::Buffer _buffer;
    tf2_ros::TransformListener _listener;

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_debug_image_pub;
    rclcpp::Publisher<mavros_msgs::msg::OpticalFlowRad>::SharedPtr m_optical_flow_rad_pub;

    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;  ///< Camera info subscriber
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr
        m_image_sub;  ///< Camera image subscriber

    // message_filters::Subscriber<sensor_msgs::msg::Image> _image_sub;
    // message_filters::Subscriber<sensor_msgs::msg::CameraInfo> _camera_info_sub;

    // std::shared_ptr<message_filters::TimeSynchronizer<sensor_msgs::msg::Image, sensor_msgs::msg::CameraInfo>> _sync;
};
