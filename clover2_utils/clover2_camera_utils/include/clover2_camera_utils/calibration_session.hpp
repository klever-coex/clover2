/**
 * @file calibration_session.hpp
 * @brief Provides state for an interactive camera calibration session.
 */

#pragma once

#include <clover2_camera_utils/calibration/algorithm/base.hpp>
#include <clover2_camera_utils/calibration/sample/selector.hpp>
#include <clover2_camera_utils/calibration/target/base.hpp>
#include <clover2_camera_utils/data/board_orientation.hpp>
#include <clover2_camera_utils/data/sample_selector_config.hpp>
#include <clover2_camera_utils_msgs/action/calibrate_camera.hpp>

#include <opencv2/core/mat.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/srv/set_camera_info.hpp>

#include <memory>
#include <optional>
#include <utility>

namespace clover2_camera_utils {

/** @brief Owns the state and image-processing logic for one calibration goal.
 */
class calibration_session {
public:
    using calibrate_camera = clover2_camera_utils_msgs::action::CalibrateCamera;
    using goal_handle = rclcpp_action::ServerGoalHandle<calibrate_camera>;
    using set_camera_info = sensor_msgs::srv::SetCameraInfo;

    /** @brief Describes an initial camera-estimate state transition. */
    enum class calibration_initialization_event {
        initial_estimate_failed,
        tilt_correction_failed,
        tilt_selection_enabled,
    };

    /** @brief Describes the result of processing one grayscale camera image. */
    struct image_processing_result {
        bool board_detected{false};
        uint8_t feedback_stage{calibrate_camera::Feedback::WAITING_FOR_BOARD};
        std::optional<cv::Mat> debug_image;
        std::optional<calibration_initialization_event> initialization_event;
    };

    explicit calibration_session(
        data::sample_selector_config sample_selector_config,
        std::shared_ptr<const calibration::target::base> target,
        std::unique_ptr<calibration::algorithm::base> algorithm);

    /**
     * @brief Detects, selects, and evaluates a calibration sample.
     * @param image Grayscale image in `CV_8UC1` format.
     * @param create_debug_image Whether to create a BGR debug image even when
     * no board is detected.
     * @return Detection, selection, and debug-rendering outcome.
     */
    image_processing_result process_image(const cv::Mat& image,
                                          bool create_debug_image);

    /** @brief Checks whether the algorithm, estimate, and dataset are ready. */
    bool ready_to_calibrate() const;

    /**
     * @brief Calculates final camera parameters from selected observations.
     * @return Calibration result, or `std::nullopt` if the session is not
     * ready or calibration fails.
     */
    std::optional<calibration::algorithm::result> run_calibration() const;

    std::shared_ptr<goal_handle> m_action_goal_handle;
    bool m_calibrating{false};
    std::shared_ptr<const calibration::target::base> m_target;
    std::unique_ptr<calibration::algorithm::base> m_algorithm;
    calibration::sample::selector m_sample_selector;
    std::optional<calibration::algorithm::result> m_camera_estimate;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_debug_publisher;
    rclcpp::Client<set_camera_info>::SharedPtr m_set_camera_info_client;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_subscription;

private:
    calibration_initialization_event initialize_camera_estimate();
    bool correct_collected_sample_tilts();
    std::optional<data::board_orientation> calculate_corrected_orientation(
        const data::observation& observation) const;

    std::optional<data::sample_selection_result> add_corrected_sample(
        data::target_detection& detection);
};

}  // namespace clover2_camera_utils
