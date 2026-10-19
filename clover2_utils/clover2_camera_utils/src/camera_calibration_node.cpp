#include <clover2_camera_utils/camera_calibration_node.hpp>

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/calib3d.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>

#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace clover2_camera_utils {
namespace {

constexpr auto k_timeout_check_period = std::chrono::milliseconds(100);
constexpr const char* k_image_message_type = "sensor_msgs/msg/Image";

}  // namespace

camera_calibration_node::camera_calibration_node(
    const rclcpp::NodeOptions& options)
    : clover2_common::node("camera_calibration", options) {
    image_timeout_s_ = declare_parameter<double>("image_timeout", 5.0);
    const auto minimum_samples = declare_parameter<int>(
        "minimum_samples_for_completion",
        static_cast<int>(
            sample_selector_config_.minimum_samples_for_completion));
    if (image_timeout_s_ <= 0.0) {
        throw std::invalid_argument("image_timeout must be positive");
    }
    if (minimum_samples < 3) {
        throw std::invalid_argument(
            "minimum_samples_for_completion must be at least 3");
    }
    sample_selector_config_.minimum_samples_for_completion =
        static_cast<size_t>(minimum_samples);

    session_callback_group_ =
        create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    using std::placeholders::_1;
    using std::placeholders::_2;
    action_server_ = rclcpp_action::create_server<calibrate_camera>(
        this, "~/calibrate",
        std::bind(&camera_calibration_node::handle_goal, this, _1, _2),
        std::bind(&camera_calibration_node::handle_cancel, this, _1),
        std::bind(&camera_calibration_node::handle_accepted, this, _1));
    finish_calibration_service_ = create_service<finish_calibration>(
        "~/finish",
        std::bind(&camera_calibration_node::handle_finish_calibration, this, _1,
                  _2),
        rclcpp::ServicesQoS(), session_callback_group_);

    RCLCPP_INFO(
        get_logger(),
        "Camera calibration Action server is ready; image timeout is %.3f s",
        image_timeout_s_);
}

camera_calibration_node::~camera_calibration_node() {
    std::scoped_lock lock(session_mutex_);
    cleanup_active_session();
    action_server_.reset();
}

rclcpp_action::GoalResponse camera_calibration_node::handle_goal(
    [[maybe_unused]] const rclcpp_action::GoalUUID& uuid,
    const std::shared_ptr<const calibrate_camera::Goal> goal) {
    std::string rejection_reason;
    if (!is_valid_goal(*goal, rejection_reason)) {
        RCLCPP_WARN(get_logger(), "Rejecting calibration goal: %s",
                    rejection_reason.c_str());
        return rclcpp_action::GoalResponse::REJECT;
    }

    if (!has_image_publisher(goal->image_topic)) {
        RCLCPP_WARN(get_logger(),
                    "Rejecting calibration goal: no %s publisher on '%s'",
                    k_image_message_type, goal->image_topic.c_str());
        return rclcpp_action::GoalResponse::REJECT;
    }

    std::scoped_lock lock(session_mutex_);
    if (goal_reserved_ || active_goal_) {
        RCLCPP_WARN(get_logger(),
                    "Rejecting calibration goal: another calibration session "
                    "is active");
        return rclcpp_action::GoalResponse::REJECT;
    }

    goal_reserved_ = true;
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse camera_calibration_node::handle_cancel(
    const std::shared_ptr<goal_handle> goal_handle) {
    if (!is_active_goal(goal_handle)) {
        return rclcpp_action::CancelResponse::REJECT;
    }
    return rclcpp_action::CancelResponse::ACCEPT;
}

void camera_calibration_node::handle_accepted(
    const std::shared_ptr<goal_handle> goal_handle) {
    const auto goal = goal_handle->get_goal();

    {
        std::scoped_lock lock(session_mutex_);
        active_goal_ = goal_handle;
        goal_reserved_ = false;
        board_.rows = goal->board_rows;
        board_.columns = goal->board_columns;
        board_.square_size_m = goal->square_size_m;
        sample_selector_ =
            calibration::sample_selector(sample_selector_config_);
        calibrating_ = false;
        last_image_received_at_ = std::chrono::steady_clock::now();

        debug_publisher_ = create_publisher<sensor_msgs::msg::Image>(
            "~/output/debug", rclcpp::SystemDefaultsQoS());
        rclcpp::SubscriptionOptions subscription_options;
        subscription_options.callback_group = session_callback_group_;
        image_subscription_ = create_subscription<sensor_msgs::msg::Image>(
            goal->image_topic, rclcpp::SensorDataQoS(),
            std::bind(&camera_calibration_node::image_callback, this,
                      std::placeholders::_1),
            subscription_options);
        image_timeout_timer_ = create_wall_timer(
            k_timeout_check_period,
            std::bind(&camera_calibration_node::check_image_timeout, this),
            session_callback_group_);
    }

    RCLCPP_INFO(get_logger(),
                "Calibration session started for image topic '%s'",
                goal->image_topic.c_str());
}

void camera_calibration_node::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr message) {
    data::board_config board;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr debug_publisher;
    std::shared_ptr<goal_handle> goal_handle;
    {
        std::scoped_lock lock(session_mutex_);
        if (!active_goal_ || calibrating_) {
            return;
        }
        last_image_received_at_ = std::chrono::steady_clock::now();
        board = board_;
        debug_publisher = debug_publisher_;
        goal_handle = active_goal_;
    }

    bool board_detected = false;
    std::optional<data::observation> observation;
    try {
        const auto input =
            cv_bridge::toCvShare(message, sensor_msgs::image_encodings::BGR8);
        observation = detector_.detect(input->image, board);
        board_detected = observation.has_value();

        if (debug_publisher && debug_publisher->get_subscription_count() > 0 &&
            is_active_goal(goal_handle)) {
            cv::Mat debug_image = input->image.clone();
            cv::drawChessboardCorners(debug_image,
                                      cv::Size(static_cast<int>(board.columns),
                                               static_cast<int>(board.rows)),
                                      observation ? observation->image_points
                                                  : std::vector<cv::Point2f>{},
                                      board_detected);

            cv_bridge::CvImage output;
            output.header = message->header;
            output.encoding = sensor_msgs::image_encodings::BGR8;
            output.image = std::move(debug_image);
            debug_publisher->publish(*output.toImageMsg());
        }
    } catch (const std::exception& error) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "Failed to process calibration image: %s",
                             error.what());
    }

    if (!is_active_goal(goal_handle)) {
        return;
    }

    uint8_t stage = board_detected
                        ? calibrate_camera::Feedback::BOARD_DETECTED
                        : calibrate_camera::Feedback::WAITING_FOR_BOARD;
    if (observation) {
        std::scoped_lock lock(session_mutex_);
        if (active_goal_ != goal_handle || calibrating_) {
            return;
        }
        const auto selection =
            sample_selector_.try_add(std::move(*observation), board);
        stage = selection.progress.data_sufficient
                    ? calibrate_camera::Feedback::DATA_SUFFICIENT
                : selection.accepted
                    ? calibrate_camera::Feedback::SAMPLE_ACCEPTED
                    : calibrate_camera::Feedback::BOARD_DETECTED;
    }
    publish_feedback(goal_handle, board_detected, stage);
}

void camera_calibration_node::check_image_timeout() {
    std::shared_ptr<goal_handle> canceling_goal;
    bool timed_out = false;
    {
        std::scoped_lock lock(session_mutex_);
        if (!active_goal_) {
            return;
        }
        if (active_goal_->is_canceling() && !calibrating_) {
            canceling_goal = active_goal_;
        }
        const auto elapsed =
            std::chrono::steady_clock::now() - last_image_received_at_;
        timed_out =
            !calibrating_ &&
            elapsed >
                std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                    std::chrono::duration<double>(image_timeout_s_));
    }

    if (canceling_goal) {
        cancel_active_session(canceling_goal);
        return;
    }

    if (timed_out) {
        abort_active_session("Timed out waiting for an image");
    }
}

void camera_calibration_node::handle_finish_calibration(
    [[maybe_unused]] const finish_calibration::Request::SharedPtr request,
    const finish_calibration::Response::SharedPtr response) {
    std::shared_ptr<goal_handle> goal_handle;
    std::vector<data::observation> observations;
    {
        std::scoped_lock lock(session_mutex_);
        if (!active_goal_) {
            response->accepted = false;
            response->message = "No active calibration session";
            return;
        }
        if (calibrating_) {
            response->accepted = false;
            response->message = "Calibration is already running";
            return;
        }
        if (!sample_selector_.progress().data_sufficient) {
            response->accepted = false;
            response->message = "Calibration dataset is not sufficient";
            return;
        }

        goal_handle = active_goal_;
        observations = sample_selector_.observations();
        calibrating_ = true;
        image_subscription_.reset();
        debug_publisher_.reset();
    }

    response->accepted = true;
    response->message = "Calibration started";
    publish_feedback(goal_handle, false,
                     calibrate_camera::Feedback::CALIBRATING);
    run_calibration(goal_handle, std::move(observations));
}

void camera_calibration_node::run_calibration(
    std::shared_ptr<goal_handle> goal_handle,
    std::vector<data::observation> observations) {
    calibration::pinhole_calibration_algorithm algorithm;
    const auto calibration_result = algorithm.calibrate(observations);

    if (goal_handle->is_canceling()) {
        cancel_active_session(goal_handle);
        return;
    }
    if (!calibration_result) {
        abort_active_session("Pinhole calibration failed");
        return;
    }

    auto result = std::make_shared<calibrate_camera::Result>();
    result->success = true;
    result->camera_info = make_camera_info(*calibration_result);
    result->rms_reprojection_error = calibration_result->rms_reprojection_error;
    result->used_observations = static_cast<uint32_t>(observations.size());

    bool cancel_after_calibration = false;
    {
        std::scoped_lock lock(session_mutex_);
        if (active_goal_ != goal_handle || !calibrating_) {
            return;
        }
        if (goal_handle->is_canceling()) {
            cancel_after_calibration = true;
        } else {
            cleanup_active_session();
        }
    }
    if (cancel_after_calibration) {
        cancel_active_session(goal_handle);
        return;
    }
    try {
        goal_handle->succeed(result);
        RCLCPP_INFO(get_logger(), "Calibration session completed successfully");
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(),
                     "Skipping success result for an inactive goal: %s",
                     error.what());
    }
}

bool camera_calibration_node::is_valid_goal(
    const calibrate_camera::Goal& goal, std::string& rejection_reason) const {
    if (goal.image_topic.empty() || goal.image_topic.front() != '/') {
        rejection_reason = "image_topic must be an absolute ROS topic name";
        return false;
    }
    if (goal.calibration_method != "pinhole") {
        rejection_reason = "only the pinhole calibration_method is supported";
        return false;
    }
    if (goal.board_type != "chessboard") {
        rejection_reason = "only the chessboard board_type is supported";
        return false;
    }
    if (goal.board_rows < 2 || goal.board_columns < 2 ||
        goal.square_size_m <= 0.0) {
        rejection_reason =
            "board_rows and board_columns must be at least 2, and "
            "square_size_m must be positive";
        return false;
    }
    return true;
}

bool camera_calibration_node::has_image_publisher(
    const std::string& topic) const {
    const auto publishers = get_publishers_info_by_topic(topic);
    return std::any_of(
        publishers.begin(), publishers.end(), [](const auto& publisher) {
            return publisher.topic_type() == k_image_message_type;
        });
}

bool camera_calibration_node::is_active_goal(
    const std::shared_ptr<goal_handle>& goal_handle) const {
    std::scoped_lock lock(session_mutex_);
    return active_goal_ == goal_handle;
}

void camera_calibration_node::publish_feedback(
    const std::shared_ptr<goal_handle>& goal_handle, const bool board_detected,
    const uint8_t stage) const {
    data::coverage_progress progress;
    size_t accepted_samples = 0;
    {
        std::scoped_lock lock(session_mutex_);
        if (active_goal_ != goal_handle) {
            return;
        }
        progress = sample_selector_.progress();
        accepted_samples = sample_selector_.accepted_count();
    }

    auto feedback = std::make_shared<calibrate_camera::Feedback>();
    feedback->accepted_samples = static_cast<uint32_t>(accepted_samples);
    feedback->progress_x = static_cast<float>(progress.x);
    feedback->progress_y = static_cast<float>(progress.y);
    feedback->progress_size = static_cast<float>(progress.size);
    feedback->progress_skew = static_cast<float>(progress.skew);
    feedback->data_sufficient = progress.data_sufficient;
    feedback->stage = stage;
    if (stage == calibrate_camera::Feedback::SAMPLE_ACCEPTED) {
        feedback->hint = "Calibration sample accepted.";
    } else if (stage == calibrate_camera::Feedback::DATA_SUFFICIENT) {
        feedback->hint = "Calibration dataset is sufficient.";
    } else if (stage == calibrate_camera::Feedback::CALIBRATING) {
        feedback->hint = "Computing camera calibration.";
    } else {
        feedback->hint = board_detected
                             ? "Chessboard detected. Evaluating the sample."
                             : "Show the configured chessboard to the camera.";
    }
    try {
        goal_handle->publish_feedback(feedback);
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(), "Skipping feedback for an inactive goal: %s",
                     error.what());
    }
}

void camera_calibration_node::cancel_active_session(
    const std::shared_ptr<goal_handle>& goal_handle) {
    {
        std::scoped_lock lock(session_mutex_);
        if (active_goal_ != goal_handle) {
            return;
        }
        cleanup_active_session();
    }

    auto result = std::make_shared<calibrate_camera::Result>();
    result->success = false;
    result->message = "Calibration canceled";
    try {
        goal_handle->canceled(result);
        RCLCPP_INFO(get_logger(), "Calibration session canceled");
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(),
                     "Skipping canceled result for an inactive goal: %s",
                     error.what());
    }
}

void camera_calibration_node::abort_active_session(const std::string& reason) {
    std::shared_ptr<goal_handle> goal_handle;
    {
        std::scoped_lock lock(session_mutex_);
        if (!active_goal_) {
            return;
        }
        goal_handle = active_goal_;
        cleanup_active_session();
    }

    auto result = std::make_shared<calibrate_camera::Result>();
    result->success = false;
    result->message = reason;
    try {
        goal_handle->abort(result);
        RCLCPP_WARN(get_logger(), "Calibration session aborted: %s",
                    reason.c_str());
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(),
                     "Skipping aborted result for an inactive goal: %s",
                     error.what());
    }
}

void camera_calibration_node::cleanup_active_session() {
    image_timeout_timer_.reset();
    image_subscription_.reset();
    debug_publisher_.reset();
    sample_selector_ = calibration::sample_selector(sample_selector_config_);
    calibrating_ = false;
    active_goal_.reset();
    goal_reserved_ = false;
}

sensor_msgs::msg::CameraInfo camera_calibration_node::make_camera_info(
    const data::calibration_result& result) {
    sensor_msgs::msg::CameraInfo camera_info;
    camera_info.width = static_cast<uint32_t>(result.image_size.width);
    camera_info.height = static_cast<uint32_t>(result.image_size.height);
    camera_info.distortion_model = "plumb_bob";
    for (size_t row = 0; row < 3; ++row) {
        for (size_t column = 0; column < 3; ++column) {
            camera_info.k[row * 3 + column] = result.camera_matrix.at<double>(
                static_cast<int>(row), static_cast<int>(column));
        }
    }
    camera_info.r[0] = 1.0;
    camera_info.r[4] = 1.0;
    camera_info.r[8] = 1.0;
    camera_info.p[0] = camera_info.k[0];
    camera_info.p[2] = camera_info.k[2];
    camera_info.p[5] = camera_info.k[4];
    camera_info.p[6] = camera_info.k[5];
    camera_info.p[10] = 1.0;
    camera_info.d.reserve(result.distortion_coefficients.total());
    for (int row = 0; row < result.distortion_coefficients.rows; ++row) {
        for (int column = 0; column < result.distortion_coefficients.cols;
             ++column) {
            camera_info.d.push_back(
                result.distortion_coefficients.at<double>(row, column));
        }
    }
    return camera_info;
}

}  // namespace clover2_camera_utils

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_camera_utils::camera_calibration_node)
