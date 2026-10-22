#include <clover2_camera_utils/camera_calibration_node.hpp>

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/calib3d.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/srv/set_camera_info.hpp>

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
constexpr const char* k_image_topic_name = "image_raw";
constexpr const char* k_set_camera_info_service_name = "set_camera_info";

}  // namespace

camera_calibration_node::camera_calibration_node(
    const rclcpp::NodeOptions& options)
    : clover2_common::node("camera_calibration", options) {
    image_timeout_s_ = declare_parameter<double>("image_timeout", 5.0);
    processing_rate_hz_ = declare_parameter<double>("processing_rate", 5.0);
    const auto minimum_samples = declare_parameter<int>(
        "minimum_samples_for_completion",
        static_cast<int>(
            sample_selector_config_.minimum_samples_for_completion));
    if (image_timeout_s_ <= 0.0) {
        throw std::invalid_argument("image_timeout must be positive");
    }
    if (processing_rate_hz_ <= 0.0) {
        throw std::invalid_argument("processing_rate must be positive");
    }
    if (minimum_samples < 3) {
        throw std::invalid_argument(
            "minimum_samples_for_completion must be at least 3");
    }
    sample_selector_config_.minimum_samples_for_completion =
        static_cast<size_t>(minimum_samples);

    session_callback_group_ =
        create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    using std::placeholders::_1;
    using std::placeholders::_2;
    action_server_ = rclcpp_action::create_server<calibrate_camera>(
        this, "~/calibrate",
        std::bind(&camera_calibration_node::handle_goal, this, _1, _2),
        std::bind(&camera_calibration_node::handle_cancel, this, _1),
        std::bind(&camera_calibration_node::handle_accepted, this, _1),
        rcl_action_server_get_default_options(), session_callback_group_);
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

    const auto image_topic =
        goal->camera_namespace == "/"
            ? std::string("/") + k_image_topic_name
            : goal->camera_namespace + "/" + k_image_topic_name;
    if (!has_image_publisher(image_topic)) {
        RCLCPP_WARN(get_logger(),
                    "Rejecting calibration goal: no %s publisher on '%s'",
                    k_image_message_type, image_topic.c_str());
        return rclcpp_action::GoalResponse::REJECT;
    }

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
    if (active_goal_ != goal_handle) {
        return rclcpp_action::CancelResponse::REJECT;
    }
    return rclcpp_action::CancelResponse::ACCEPT;
}

void camera_calibration_node::handle_accepted(
    const std::shared_ptr<goal_handle> goal_handle) {
    const auto goal = goal_handle->get_goal();
    const auto detector =
        calibration::board_detector_factory::instance().create(
            goal->board_type);
    const auto image_topic =
        goal->camera_namespace == "/"
            ? std::string("/") + k_image_topic_name
            : goal->camera_namespace + "/" + k_image_topic_name;
    const auto set_camera_info_service =
        goal->camera_namespace == "/"
            ? std::string("/") + k_set_camera_info_service_name
            : goal->camera_namespace + "/" + k_set_camera_info_service_name;

    active_goal_ = goal_handle;
    goal_reserved_ = false;
    board_.rows = goal->board_rows;
    board_.columns = goal->board_columns;
    board_.square_size_m = goal->square_size_m;
    detector_ = detector;
    sample_selector_ = calibration::sample_selector(sample_selector_config_);
    calibrating_ = false;
    last_image_received_at_ = std::chrono::steady_clock::now();
    last_image_processed_at_ = std::chrono::steady_clock::time_point{};

    const auto image_qos = rclcpp::SensorDataQoS().keep_last(1);
    debug_publisher_ = create_publisher<sensor_msgs::msg::Image>(
        "~/output/debug", rclcpp::SystemDefaultsQoS());
    set_camera_info_client_ = create_client<set_camera_info>(
        set_camera_info_service, rclcpp::ServicesQoS(),
        session_callback_group_);
    rclcpp::SubscriptionOptions subscription_options;
    subscription_options.callback_group = session_callback_group_;
    image_subscription_ = create_subscription<sensor_msgs::msg::Image>(
        image_topic, image_qos,
        std::bind(&camera_calibration_node::image_callback, this,
                  std::placeholders::_1),
        subscription_options);
    image_timeout_timer_ = create_wall_timer(
        k_timeout_check_period,
        std::bind(&camera_calibration_node::check_image_timeout, this),
        session_callback_group_);

    RCLCPP_INFO(get_logger(),
                "Calibration session started for image topic '%s'; calibration "
                "will be stored through '%s'",
                image_topic.c_str(), set_camera_info_service.c_str());
}

void camera_calibration_node::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr message) {
    if (!active_goal_ || calibrating_) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    last_image_received_at_ = now;
    const auto processing_period =
        std::chrono::duration<double>(1.0 / processing_rate_hz_);
    if (last_image_processed_at_ != std::chrono::steady_clock::time_point{} &&
        now - last_image_processed_at_ < processing_period) {
        return;
    }
    last_image_processed_at_ = now;

    bool board_detected = false;
    std::optional<data::observation> observation;
    try {
        const auto input =
            cv_bridge::toCvShare(message, sensor_msgs::image_encodings::BGR8);
        observation = detector_->detect(input->image, board_);
        board_detected = observation.has_value();

        if (debug_publisher_ &&
            debug_publisher_->get_subscription_count() > 0) {
            cv::Mat debug_image = input->image.clone();
            cv::drawChessboardCorners(debug_image,
                                      cv::Size(static_cast<int>(board_.columns),
                                               static_cast<int>(board_.rows)),
                                      observation ? observation->image_points
                                                  : std::vector<cv::Point2f>{},
                                      board_detected);

            cv_bridge::CvImage output;
            output.header = message->header;
            output.encoding = sensor_msgs::image_encodings::BGR8;
            output.image = std::move(debug_image);
            debug_publisher_->publish(*output.toImageMsg());
        }
    } catch (const std::exception& error) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "Failed to process calibration image: %s",
                             error.what());
    }

    uint8_t stage = board_detected
                        ? calibrate_camera::Feedback::BOARD_DETECTED
                        : calibrate_camera::Feedback::WAITING_FOR_BOARD;
    if (observation) {
        const auto selection =
            sample_selector_.try_add(std::move(*observation), board_);
        stage = selection.progress.data_sufficient
                    ? calibrate_camera::Feedback::DATA_SUFFICIENT
                : selection.accepted
                    ? calibrate_camera::Feedback::SAMPLE_ACCEPTED
                    : calibrate_camera::Feedback::BOARD_DETECTED;
    }
    publish_feedback(active_goal_, board_detected, stage);
}

void camera_calibration_node::check_image_timeout() {
    bool timed_out = false;
    if (!active_goal_) {
        return;
    }
    if (active_goal_->is_canceling()) {
        cancel_active_session(active_goal_);
        return;
    }
    const auto elapsed =
        std::chrono::steady_clock::now() - last_image_received_at_;
    timed_out =
        !calibrating_ &&
        elapsed >
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(image_timeout_s_));

    if (timed_out) {
        abort_active_session("Timed out waiting for an image");
    }
}

void camera_calibration_node::handle_finish_calibration(
    [[maybe_unused]] const finish_calibration::Request::SharedPtr request,
    const finish_calibration::Response::SharedPtr response) {
    std::shared_ptr<goal_handle> goal_handle;
    std::vector<data::observation> observations;
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

    response->accepted = true;
    response->message = "Calibration started";
    publish_feedback(goal_handle, false,
                     calibrate_camera::Feedback::CALIBRATING);
    run_calibration(goal_handle, std::move(observations));
}

void camera_calibration_node::run_calibration(
    std::shared_ptr<goal_handle> goal_handle,
    std::vector<data::observation> observations) {
    const auto algorithm = calibration::factory::instance().create(
        goal_handle->get_goal()->calibration_method);
    const auto calibration_result = algorithm->calibrate(observations);

    if (!calibration_result) {
        abort_active_session("Camera calibration failed");
        return;
    }

    auto result = std::make_shared<calibrate_camera::Result>();
    result->success = true;
    result->camera_info = make_camera_info(*calibration_result);
    result->used_observations = static_cast<uint32_t>(observations.size());

    if (active_goal_ != goal_handle || !calibrating_) {
        return;
    }
    if (!set_camera_info_client_ ||
        !set_camera_info_client_->service_is_ready()) {
        abort_active_session("Camera set_camera_info service is not available");
        return;
    }

    auto request = std::make_shared<set_camera_info::Request>();
    request->camera_info = result->camera_info;
    try {
        set_camera_info_client_->async_send_request(
            request, [this, goal_handle, result](
                         rclcpp::Client<set_camera_info>::SharedFuture future) {
                handle_set_camera_info_response(goal_handle, result,
                                                std::move(future));
            });
    } catch (const std::runtime_error& error) {
        abort_active_session(
            std::string("Failed to call camera set_camera_info "
                        "service: ") +
            error.what());
    }
}

void camera_calibration_node::handle_set_camera_info_response(
    const std::shared_ptr<goal_handle>& goal_handle,
    const std::shared_ptr<calibrate_camera::Result>& result,
    rclcpp::Client<set_camera_info>::SharedFuture future) {
    std::shared_ptr<set_camera_info::Response> response;
    try {
        response = future.get();
    } catch (const std::exception& error) {
        abort_active_session(
            std::string("Camera set_camera_info service failed: ") +
            error.what());
        return;
    }

    if (goal_handle->is_canceling()) {
        cancel_active_session(goal_handle);
        return;
    }
    if (!response->success) {
        const auto reason = response->status_message.empty()
                                ? "Camera rejected calibration data"
                                : response->status_message;
        abort_active_session("Camera set_camera_info failed: " + reason);
        return;
    }

    if (active_goal_ != goal_handle || !calibrating_) {
        return;
    }
    cleanup_active_session();
    try {
        goal_handle->succeed(result);
        RCLCPP_INFO(get_logger(),
                    "Calibration session completed and stored by the camera");
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(),
                     "Skipping success result for an inactive goal: %s",
                     error.what());
    }
}

bool camera_calibration_node::is_valid_goal(
    const calibrate_camera::Goal& goal, std::string& rejection_reason) const {
    if (goal.camera_namespace.empty() || goal.camera_namespace.front() != '/') {
        rejection_reason = "camera_namespace must be an absolute ROS namespace";
        return false;
    }
    if (!calibration::factory::instance().contains(goal.calibration_method)) {
        rejection_reason =
            "unsupported calibration_method '" + goal.calibration_method + "'";
        return false;
    }
    if (!calibration::board_detector_factory::instance().contains(
            goal.board_type)) {
        rejection_reason = "unsupported board_type '" + goal.board_type + "'";
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

void camera_calibration_node::publish_feedback(
    const std::shared_ptr<goal_handle>& goal_handle, const bool board_detected,
    const uint8_t stage) const {
    data::coverage_progress progress;
    size_t accepted_samples = 0;
    if (active_goal_ != goal_handle) {
        return;
    }
    progress = sample_selector_.progress();
    accepted_samples = sample_selector_.accepted_count();

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
    if (active_goal_ != goal_handle) {
        return;
    }
    cleanup_active_session();

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
    if (!active_goal_) {
        return;
    }
    const auto goal_handle = active_goal_;
    cleanup_active_session();

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
    set_camera_info_client_.reset();
    detector_.reset();
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
    camera_info.distortion_model = result.distortion_model;
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
