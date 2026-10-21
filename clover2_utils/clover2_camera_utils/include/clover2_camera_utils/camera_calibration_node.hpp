#pragma once
#include <clover2_camera_utils/calibration/board_detector.hpp>
#include <clover2_camera_utils/calibration/board_detector_factory.hpp>
#include <clover2_camera_utils/calibration/factory.hpp>
#include <clover2_camera_utils/calibration/sample_selector.hpp>
#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/calibration_result.hpp>
#include <clover2_camera_utils/data/observation.hpp>
#include <clover2_camera_utils/data/sample_selector_config.hpp>
#include <clover2_camera_utils_msgs/action/calibrate_camera.hpp>
#include <clover2_camera_utils_msgs/srv/finish_calibration.hpp>
#include <clover2_common/node.hpp>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/srv/set_camera_info.hpp>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace clover2_camera_utils {

class camera_calibration_node : public clover2_common::node {
public:
    using calibrate_camera = clover2_camera_utils_msgs::action::CalibrateCamera;
    using goal_handle = rclcpp_action::ServerGoalHandle<calibrate_camera>;
    using finish_calibration =
        clover2_camera_utils_msgs::srv::FinishCalibration;
    using set_camera_info = sensor_msgs::srv::SetCameraInfo;

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
    void handle_finish_calibration(
        const finish_calibration::Request::SharedPtr request,
        finish_calibration::Response::SharedPtr response);
    void run_calibration(std::shared_ptr<goal_handle> goal_handle,
                         std::vector<data::observation> observations);
    void handle_set_camera_info_response(
        const std::shared_ptr<goal_handle>& goal_handle,
        const std::shared_ptr<calibrate_camera::Result>& result,
        rclcpp::Client<set_camera_info>::SharedFuture future);

    bool is_valid_goal(const calibrate_camera::Goal& goal,
                       std::string& rejection_reason) const;
    bool has_image_publisher(const std::string& topic) const;
    bool is_active_goal(const std::shared_ptr<goal_handle>& goal_handle) const;
    void publish_feedback(const std::shared_ptr<goal_handle>& goal_handle,
                          bool board_detected, uint8_t stage) const;
    void cancel_active_session(const std::shared_ptr<goal_handle>& goal_handle);
    void abort_active_session(const std::string& reason);
    void cleanup_active_session();
    static sensor_msgs::msg::CameraInfo make_camera_info(
        const data::calibration_result& result);

    mutable std::mutex session_mutex_;
    bool goal_reserved_{false};
    double image_timeout_s_{5.0};
    double processing_rate_hz_{5.0};
    data::sample_selector_config sample_selector_config_;

    data::board_config board_;
    std::shared_ptr<const calibration::board_detector> detector_;
    calibration::sample_selector sample_selector_;
    bool calibrating_{false};
    rclcpp_action::Server<calibrate_camera>::SharedPtr action_server_;
    rclcpp::CallbackGroup::SharedPtr session_callback_group_;
    rclcpp::Service<finish_calibration>::SharedPtr finish_calibration_service_;
    std::shared_ptr<goal_handle> active_goal_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr debug_publisher_;
    rclcpp::Client<set_camera_info>::SharedPtr set_camera_info_client_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr
        image_subscription_;
    rclcpp::TimerBase::SharedPtr image_timeout_timer_;
    std::chrono::steady_clock::time_point last_image_received_at_;
    std::chrono::steady_clock::time_point last_image_processed_at_;
};

}  // namespace clover2_camera_utils
