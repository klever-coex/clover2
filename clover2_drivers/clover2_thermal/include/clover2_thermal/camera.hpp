#pragma once

#include <clover2_common/node.hpp>
#include <opencv2/videoio.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <atomic>
#include <thread>

namespace clover2_thermal {

class camera : public clover2_common::node {
public:
    explicit camera(const rclcpp::NodeOptions& options);
    ~camera() override;

private:
    void capture_loop();

    cv::VideoCapture m_capture;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_viz_publisher;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_temperature_publisher;
    std::atomic<bool> m_running{true};
    std::thread m_capture_thread;
};

}  // namespace clover2_thermal
