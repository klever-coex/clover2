#include <optical_flow/optical_flow.hpp>

OpticalFlow::OpticalFlow(const rclcpp::NodeOptions& options) 
: Node("optical_flow", options),
_buffer(this->get_clock()),
_listener(_buffer),
m_prev_stamp(this->get_clock()->now())
{

    this->declare_parameter("roi_size", 128);
    _roi_size = this->get_parameter("roi_size").as_int();

    m_debug_image_pub = create_publisher<sensor_msgs::msg::Image>("~/debug_image", 10);
    m_optical_flow_rad_pub = create_publisher<mavros_msgs::msg::OpticalFlowRad>("~/optical_flow", 10);

    m_camera_info_sub = this->create_subscription<sensor_msgs::msg::CameraInfo>(
        "~/camera_info", rclcpp::SensorDataQoS(),
        std::bind(&OpticalFlow::camera_info_callback, this,
                  std::placeholders::_1));
}

void OpticalFlow::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) {
    // std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    // validate camera info
    if (msg->height == 0 || msg->width == 0 || msg->d.size() == 0) {
        return;
    }

    m_camera_model.fromCameraInfo(msg);
}


void OpticalFlow::flow_calc(const sensor_msgs::msg::Image::ConstSharedPtr& msg, const sensor_msgs::msg::CameraInfo::ConstSharedPtr& cinfo)
{
    // if ((rclcpp::Time(msg->header.stamp) - m_prev_stamp).seconds() < 0.1)
    // {
    //     return;
    // }

    auto img = cv_bridge::toCvShare(msg, "mono8")->image;

    if (_roi.empty())
    {
        _roi = cv::Rect(int((msg->width - _roi_size)/2), int((msg->height - _roi_size)/2), _roi_size, _roi_size);
    }

    img = img(_roi);

    img.convertTo(_curr, CV_32F);

    if (_prev.empty() || (rclcpp::Time(msg->header.stamp) - m_prev_stamp).seconds() > 0.2)
    {
        m_prev_stamp = rclcpp::Time(msg->header.stamp);
        _prev = _curr.clone();
        cv::createHanningWindow(_hann, _curr.size(), CV_32F);
    } else {
        double quality;
        uint32_t image_center_x = msg->width / 2;
		uint32_t image_center_y = msg->height / 2;

        cv::Point2f shift = cv::phaseCorrelate(_prev, _curr, _hann, &quality);
        shift += cv::Point2f(image_center_x, image_center_y);

        std::vector<cv::Point2f> distorted_points = {shift};
        std::vector<cv::Point2f> undistorted_points(1);

        auto camera_matrix = m_camera_model.fullIntrinsicMatrix();
        auto dist_vector = m_camera_model.distortionCoeffs();
        cv::undistortPoints(distorted_points, undistorted_points, camera_matrix, dist_vector, cv::noArray(), camera_matrix);
        undistorted_points[0].x -= image_center_x;
        undistorted_points[0].y -= image_center_y;

        // double focal_length_x = camera_matrix.at<double>(0, 0);
		// double focal_length_y = camera_matrix.at<double>(1, 1);
        double focal_length_x = 1.0;
        double focal_length_y = 1.0;
        double flow_x = atan2(undistorted_points[0].x, focal_length_x);
        double flow_y = atan2(undistorted_points[0].y, focal_length_y);

        geometry_msgs::msg::Vector3Stamped camera_flow, fcu_flow;
        camera_flow.header.frame_id = msg->header.frame_id;
        camera_flow.header.stamp = msg->header.stamp;
        camera_flow.vector.x = flow_y;
        camera_flow.vector.y = -flow_x;

        try {
            fcu_flow = _buffer.transform(camera_flow, "base_link");
        } catch (const tf2::TransformException& e) {
            // transform is not available yet
            RCLCPP_ERROR(this->get_logger(), "%s", e.what());
            return;
		}

        mavros_msgs::msg::OpticalFlowRad flow;

        flow.header.stamp = msg->header.stamp;
        flow.integration_time_us = (uint32_t)((rclcpp::Time(msg->header.stamp) - m_prev_stamp).nanoseconds() / 1e3);
        flow.integrated_x = fcu_flow.vector.x;
        flow.integrated_y = fcu_flow.vector.y;
        flow.integrated_xgyro = NAN;
        flow.integrated_zgyro = NAN;
        flow.integrated_ygyro = NAN;
        flow.quality = (uint8_t)(quality * 255);

        m_optical_flow_rad_pub->publish(flow);

        _prev = _curr.clone();
        m_prev_stamp = rclcpp::Time(msg->header.stamp);

        if (m_debug_image_pub->get_subscription_count() > 0) {
            auto debug = cv_bridge::toCvShare(msg, "rgb8")->image;
            debug = debug(_roi);
            drawFlow(debug, undistorted_points[0], quality);

            sensor_msgs::msg::Image::SharedPtr debug_msg = cv_bridge::CvImage(std_msgs::msg::Header(), "RGB8", debug).toImageMsg();
            debug_msg->header.frame_id = msg->header.frame_id;
            debug_msg->header.stamp = msg->header.stamp;
            debug_msg->encoding = sensor_msgs::image_encodings::RGB8;

            m_debug_image_pub->publish(*debug_msg.get());
        }
    }
}

void OpticalFlow::drawFlow(cv::Mat& frame, cv::Point2f shift, double quality)
{
    double radius = std::sqrt(shift.x * shift.x + shift.y * shift.y);

    cv::Point2f center(frame.cols/2, frame.rows/2);

    double brightness = (1 - quality) * 25;
    cv::Scalar color(brightness, brightness, brightness);
    cv::line(frame, center, shift + center, color, 2, cv::LINE_AA);
}

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(OpticalFlow)
