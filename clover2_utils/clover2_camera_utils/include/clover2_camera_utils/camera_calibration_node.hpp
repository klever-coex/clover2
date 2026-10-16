#pragma once
#include <clover2_camera_utils/calibration/chessboard_detector.hpp>
#include <clover2_camera_utils/calibration/pinhole_calibration_algorithm.hpp>
#include <clover2_camera_utils/calibration/sample_selector.hpp>
#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/calibration_result.hpp>
#include <clover2_camera_utils/data/observation.hpp>
#include <clover2_camera_utils/data/sample_selector_config.hpp>
#include <clover2_camera_utils_msgs/action/calibrate_camera.hpp>
#include <clover2_camera_utils_msgs/srv/capture_calibration_sample.hpp>
#include <clover2_camera_utils_msgs/srv/finish_calibration.hpp>
#include <clover2_common/node.hpp>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace clover2_camera_utils {

class camera_calibration_node : public clover2_common::node {
public:
    using calibrate_camera = clover2_camera_utils_msgs::action::CalibrateCamera;
    using goal_handle = rclcpp_action::ServerGoalHandle<calibrate_camera>;
    using capture_calibration_sample =
        clover2_camera_utils_msgs::srv::CaptureCalibrationSample;
    using finish_calibration =
        clover2_camera_utils_msgs::srv::FinishCalibration;

    explicit camera_calibration_node(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~camera_calibration_node() override;

private:
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID& uuid,
        std::shared_ptr<const calibrate_camera::Goal> goal);
    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<goal_handle> goal_handle);
    void handle_accepted(const std::shared_ptr<goal_handle> goal_handle);

    void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr message);
    void check_image_timeout();
    void handle_capture_sample(
        const capture_calibration_sample::Request::SharedPtr request,
        capture_calibration_sample::Response::SharedPtr response);
    void handle_capture_timeout();
    void handle_finish_calibration(
        const finish_calibration::Request::SharedPtr request,
        finish_calibration::Response::SharedPtr response);
    void run_calibration(std::shared_ptr<goal_handle> goal_handle,
                         std::vector<data::observation> observations);

    bool is_valid_goal(const calibrate_camera::Goal& goal,
                       std::string& rejection_reason) const;
    bool has_image_publisher(const std::string& topic) const;
    bool is_active_goal(const std::shared_ptr<goal_handle>& goal_handle) const;
    void publish_feedback(const std::shared_ptr<goal_handle>& goal_handle,
                          bool board_detected, uint8_t stage) const;
    void cancel_active_session(const std::shared_ptr<goal_handle>& goal_handle);
    void abort_active_session(const std::string& reason);
    void cleanup_active_session();
    void complete_capture_locked(bool accepted, std::string message);
    static sensor_msgs::msg::CameraInfo make_camera_info(
        const data::calibration_result& result);

    mutable std::mutex session_mutex_;
    std::condition_variable capture_condition_;
    bool goal_reserved_{false};
    double image_timeout_s_{5.0};
    double capture_timeout_s_{2.0};
    data::sample_selector_config sample_selector_config_;

    data::board_config board_;
    clover2_camera_utils::calibration::chessboard_detector detector_;
    calibration::sample_selector sample_selector_;
    bool calibrating_{false};
    bool capture_pending_{false};
    bool capture_completed_{false};
    bool capture_accepted_{false};
    std::string capture_message_;
    rclcpp_action::Server<calibrate_camera>::SharedPtr action_server_;
    rclcpp::CallbackGroup::SharedPtr session_callback_group_;
    rclcpp::Service<capture_calibration_sample>::SharedPtr
        capture_sample_service_;
    rclcpp::Service<finish_calibration>::SharedPtr finish_calibration_service_;
    std::shared_ptr<goal_handle> active_goal_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr debug_publisher_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr
        image_subscription_;
    rclcpp::TimerBase::SharedPtr image_timeout_timer_;
    rclcpp::TimerBase::SharedPtr capture_timeout_timer_;
    std::thread calibration_worker_;
    std::chrono::steady_clock::time_point last_image_received_at_;
};

}  // namespace clover2_camera_utils
