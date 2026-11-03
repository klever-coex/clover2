/**
 * @file calibration_session.hpp
 * @brief Provides state for an interactive camera calibration session.
 */

#pragma once

#include <clover2_camera_utils/calibration/sample/selector.hpp>
#include <clover2_camera_utils/calibration/target/base.hpp>
#include <clover2_camera_utils/data/sample_selector_config.hpp>
#include <clover2_camera_utils_msgs/action/calibrate_camera.hpp>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/srv/set_camera_info.hpp>

#include <chrono>
#include <memory>
#include <utility>

namespace clover2_camera_utils {

/** @brief Stores resources and state for one camera calibration action goal. */
struct calibration_session {
    using calibrate_camera = clover2_camera_utils_msgs::action::CalibrateCamera;
    using goal_handle = rclcpp_action::ServerGoalHandle<calibrate_camera>;
    using set_camera_info = sensor_msgs::srv::SetCameraInfo;

    std::shared_ptr<goal_handle> action_goal_handle;
    bool calibrating{false};
    std::shared_ptr<const calibration::target::base> target;
    calibration::sample::selector sample_selector;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr debug_publisher;
    rclcpp::Client<set_camera_info>::SharedPtr set_camera_info_client;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_subscription;
    std::chrono::steady_clock::time_point last_image_processed_at;

    explicit calibration_session(
        data::sample_selector_config sample_selector_config)
        : sample_selector(std::move(sample_selector_config)) {}
};

}  // namespace clover2_camera_utils
