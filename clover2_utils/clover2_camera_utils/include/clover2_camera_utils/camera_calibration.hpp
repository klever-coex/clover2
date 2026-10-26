/**
 * @file camera_calibration.hpp
 * @brief Provides a ROS component for camera calibration.
 */

#pragma once
#include <clover2_camera_utils/calibration/algorithm/factory.hpp>
#include <clover2_camera_utils/calibration/detector/base.hpp>
#include <clover2_camera_utils/calibration/detector/factory.hpp>
#include <clover2_camera_utils/calibration/sample/selector.hpp>
#include <clover2_camera_utils/data/board_config.hpp>
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
#include <string>

namespace clover2_camera_utils {

/**
 * @brief Performs interactive camera calibration using board observations.
 *
 * The component accepts an action goal, receives images from the camera
 * namespace, selects non-duplicate observations, and sends the resulting
 * `CameraInfo` to the camera driver through the `set_camera_info` service.
 */
class camera_calibration : public clover2_common::node {
public:
    using calibrate_camera = clover2_camera_utils_msgs::action::CalibrateCamera;
    using goal_handle = rclcpp_action::ServerGoalHandle<calibrate_camera>;
    using finish_calibration =
        clover2_camera_utils_msgs::srv::FinishCalibration;
    using set_camera_info = sensor_msgs::srv::SetCameraInfo;

    /**
     * @brief Constructs the camera calibration component.
     * @param options ROS node construction options.
     */
    explicit camera_calibration(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    /** @brief Ends the active session and releases its resources. */
    ~camera_calibration() override;

private:
    /**
     * @brief Validates and accepts a new action goal.
     * @param uuid Unique goal identifier.
     * @param goal Requested calibration parameters.
     * @return Decision to accept or reject the goal.
     */
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID& uuid,
        std::shared_ptr<const calibrate_camera::Goal> goal);

    /**
     * @brief Handles a request to cancel the active goal.
     * @param goal_handle Handle of the goal to cancel.
     * @return Decision to accept or reject cancellation.
     */
    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<goal_handle> goal_handle);

    /**
     * @brief Starts processing an accepted goal.
     * @param goal_handle Handle of the accepted goal.
     */
    void handle_accepted(const std::shared_ptr<goal_handle> goal_handle);

    /**
     * @brief Processes an image during the active calibration session.
     * @param message Received image message.
     */
    void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr message);

    /**
     * @brief Handles a request to finish the active calibration session.
     * @param request Request of the `finish` service.
     * @param response Response of the `finish` service.
     */
    void handle_finish_calibration(
        const finish_calibration::Request::SharedPtr request,
        finish_calibration::Response::SharedPtr response);

    /**
     * @brief Performs calibration using selected observations.
     * @param goal_handle Handle of the active goal.
     * @param observations Calibration dataset observations.
     */
    void run_calibration(std::shared_ptr<goal_handle> goal_handle,
                         std::vector<data::observation> observations);

    /**
     * @brief Validates calibration goal parameters.
     * @param goal Goal to validate.
     * @param rejection_reason Rejection reason for an invalid goal.
     * @return `true` if the goal is valid, otherwise `false`.
     */
    bool is_valid_goal(const calibrate_camera::Goal& goal,
                       std::string& rejection_reason) const;

    /**
     * @brief Publishes feedback for the active goal.
     * @param goal_handle Handle of the active goal.
     * @param board_detected Whether a board was detected in the last image.
     * @param stage Current calibration stage.
     */
    void publish_feedback(const std::shared_ptr<goal_handle>& goal_handle,
                          bool board_detected, uint8_t stage) const;

    /**
     * @brief Cancels the active session for the specified goal.
     * @param goal_handle Handle of the goal to cancel.
     */
    void cancel_active_session(const std::shared_ptr<goal_handle>& goal_handle);

    /**
     * @brief Aborts the active session with the specified reason.
     * @param reason Session abort reason.
     */
    void abort_active_session(const std::string& reason);

    /** @brief Releases resources of the active calibration session. */
    void cleanup_active_session();

    // Configuration
    double m_processing_rate_hz{5.0};
    data::sample_selector_config m_sample_selector_config;

    // Active calibration session
    bool m_goal_reserved{false};
    bool m_calibrating{false};
    std::shared_ptr<goal_handle> m_active_goal;
    data::board_config m_board;
    std::shared_ptr<const calibration::detector::base> m_detector;
    calibration::sample::selector m_sample_selector;

    // ROS interfaces
    rclcpp_action::Server<calibrate_camera>::SharedPtr m_action_server;
    rclcpp::CallbackGroup::SharedPtr m_session_callback_group;
    rclcpp::Service<finish_calibration>::SharedPtr m_finish_calibration_service;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_debug_publisher;
    rclcpp::Client<set_camera_info>::SharedPtr m_set_camera_info_client;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr
        m_image_subscription;

    // Image processing timestamp
    std::chrono::steady_clock::time_point m_last_image_processed_at;
};

}  // namespace clover2_camera_utils
