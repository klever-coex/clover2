#include <clover2_camera_utils/camera_calibration.hpp>
#include <clover2_camera_utils/io/camera_info_yaml.hpp>

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
#include <iomanip>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>

namespace clover2_camera_utils {
namespace {

constexpr const char* k_image_topic_name = "image_raw";
constexpr const char* k_set_camera_info_service_name = "set_camera_info";

}  // namespace

camera_calibration::camera_calibration(const rclcpp::NodeOptions& options)
    : clover2_common::node("camera_calibration", options) {
    m_calibration_file =
        declare_parameter<std::string>("calibration_file", "camera_calibration.yaml");
    m_processing_rate_hz = declare_parameter<double>("processing_rate", 5.0);
    const auto minimum_samples = declare_parameter<int>(
        "minimum_samples_for_completion",
        static_cast<int>(
            m_sample_selector_config.minimum_samples_for_completion));
    if (m_calibration_file.empty()) {
        throw std::invalid_argument("calibration_file must not be empty");
    }
    if (m_processing_rate_hz <= 0.0) {
        throw std::invalid_argument("processing_rate must be positive");
    }
    if (minimum_samples < 3) {
        throw std::invalid_argument(
            "minimum_samples_for_completion must be at least 3");
    }
    m_sample_selector_config.minimum_samples_for_completion =
        static_cast<size_t>(minimum_samples);

    m_session_callback_group =
        create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    using std::placeholders::_1;
    using std::placeholders::_2;
    m_action_server = rclcpp_action::create_server<calibrate_camera>(
        this, "~/calibrate",
        std::bind(&camera_calibration::handle_goal, this, _1, _2),
        std::bind(&camera_calibration::handle_cancel, this, _1),
        std::bind(&camera_calibration::handle_accepted, this, _1),
        rcl_action_server_get_default_options(), m_session_callback_group);
    m_finish_calibration_service = create_service<finish_calibration>(
        "~/finish",
        std::bind(&camera_calibration::handle_finish_calibration, this, _1, _2),
        rclcpp::ServicesQoS(), m_session_callback_group);

    RCLCPP_INFO(
        get_logger(),
        "Camera calibration Action server is ready; results will be saved to '%s'",
        m_calibration_file.c_str());
}

camera_calibration::~camera_calibration() {
    cleanup_active_session();
    m_action_server.reset();
}

rclcpp_action::GoalResponse camera_calibration::handle_goal(
    [[maybe_unused]] const rclcpp_action::GoalUUID& uuid,
    const std::shared_ptr<const calibrate_camera::Goal> goal) {
    std::string rejection_reason;
    if (!is_valid_goal(*goal, rejection_reason)) {
        RCLCPP_WARN(get_logger(), "Rejecting calibration goal: %s",
                    rejection_reason.c_str());
        return rclcpp_action::GoalResponse::REJECT;
    }

    if (m_goal_reserved || m_active_goal) {
        RCLCPP_WARN(get_logger(),
                    "Rejecting calibration goal: another calibration session "
                    "is active");
        return rclcpp_action::GoalResponse::REJECT;
    }

    m_goal_reserved = true;
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse camera_calibration::handle_cancel(
    const std::shared_ptr<goal_handle> goal_handle) {
    if (m_active_goal != goal_handle) {
        return rclcpp_action::CancelResponse::REJECT;
    }
    return rclcpp_action::CancelResponse::ACCEPT;
}

void camera_calibration::handle_accepted(
    const std::shared_ptr<goal_handle> goal_handle) {
    const auto goal = goal_handle->get_goal();
    const auto image_topic =
        goal->camera_namespace == "/"
            ? std::string("/") + k_image_topic_name
            : goal->camera_namespace + "/" + k_image_topic_name;
    const auto set_camera_info_service =
        goal->camera_namespace == "/"
            ? std::string("/") + k_set_camera_info_service_name
            : goal->camera_namespace + "/" + k_set_camera_info_service_name;

    m_active_goal = goal_handle;
    m_goal_reserved = false;
    m_board.rows = goal->board_rows;
    m_board.columns = goal->board_columns;
    m_board.square_size_m = goal->square_size_m;
    m_detector =
        calibration::detector::factory::instance().create(goal->board_type);
    m_sample_selector = calibration::sample::selector(m_sample_selector_config);
    m_calibrating = false;
    m_last_image_processed_at = std::chrono::steady_clock::time_point{};

    const auto image_qos = rclcpp::SensorDataQoS().keep_last(1);
    m_debug_publisher = create_publisher<sensor_msgs::msg::Image>(
        "~/output/debug", rclcpp::SystemDefaultsQoS());
    m_set_camera_info_client = create_client<set_camera_info>(
        set_camera_info_service, rclcpp::ServicesQoS(),
        m_session_callback_group);
    rclcpp::SubscriptionOptions subscription_options;
    subscription_options.callback_group = m_session_callback_group;
    m_image_subscription = create_subscription<sensor_msgs::msg::Image>(
        image_topic, image_qos,
        std::bind(&camera_calibration::image_callback, this,
                  std::placeholders::_1),
        subscription_options);

    RCLCPP_INFO(get_logger(),
                "Calibration session started for image topic '%s'; results will "
                "be saved to '%s' and sent to '%s' when available",
                image_topic.c_str(), m_calibration_file.c_str(),
                set_camera_info_service.c_str());
}

void camera_calibration::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr message) {
    if (!m_active_goal || m_calibrating) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    const auto processing_period =
        std::chrono::duration<double>(1.0 / m_processing_rate_hz);
    if (m_last_image_processed_at != std::chrono::steady_clock::time_point{} &&
        now - m_last_image_processed_at < processing_period) {
        return;
    }
    m_last_image_processed_at = now;

    bool board_detected = false;
    std::optional<data::observation> observation;
    try {
        const auto input =
            cv_bridge::toCvShare(message, sensor_msgs::image_encodings::BGR8);
        observation = m_detector->detect(input->image, m_board);
        board_detected = observation.has_value();

        if (m_debug_publisher &&
            m_debug_publisher->get_subscription_count() > 0) {
            cv::Mat debug_image = input->image.clone();
            cv::drawChessboardCorners(
                debug_image,
                cv::Size(static_cast<int>(m_board.columns),
                         static_cast<int>(m_board.rows)),
                observation ? observation->image_points
                            : std::vector<cv::Point2f>{},
                board_detected);

            cv_bridge::CvImage output;
            output.header = message->header;
            output.encoding = sensor_msgs::image_encodings::BGR8;
            output.image = std::move(debug_image);
            m_debug_publisher->publish(*output.toImageMsg());
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
            m_sample_selector.try_add(std::move(*observation), m_board);
        stage = selection.progress.data_sufficient
                    ? calibrate_camera::Feedback::DATA_SUFFICIENT
                : selection.accepted
                    ? calibrate_camera::Feedback::SAMPLE_ACCEPTED
                    : calibrate_camera::Feedback::BOARD_DETECTED;
    }
    publish_feedback(m_active_goal, board_detected, stage);
}

void camera_calibration::handle_finish_calibration(
    [[maybe_unused]] const finish_calibration::Request::SharedPtr request,
    const finish_calibration::Response::SharedPtr response) {
    std::shared_ptr<goal_handle> goal_handle;
    std::vector<data::observation> observations;
    if (!m_active_goal) {
        response->accepted = false;
        response->message = "No active calibration session";
        return;
    }
    if (m_calibrating) {
        response->accepted = false;
        response->message = "Calibration is already running";
        return;
    }
    if (!m_sample_selector.progress().data_sufficient) {
        response->accepted = false;
        response->message = "Calibration dataset is not sufficient";
        return;
    }

    goal_handle = m_active_goal;
    observations = m_sample_selector.observations();
    m_calibrating = true;
    m_image_subscription.reset();
    m_debug_publisher.reset();

    response->accepted = true;
    response->message = "Calibration started";
    publish_feedback(goal_handle, false,
                     calibrate_camera::Feedback::CALIBRATING);
    run_calibration(goal_handle, std::move(observations));
}

void camera_calibration::run_calibration(
    std::shared_ptr<goal_handle> goal_handle,
    std::vector<data::observation> observations) {
    const auto algorithm = calibration::algorithm::factory::instance().create(
        goal_handle->get_goal()->calibration_method);
    const auto camera_info = algorithm->calibrate(observations);

    if (!camera_info) {
        abort_active_session("Camera calibration failed");
        return;
    }

    auto result = std::make_shared<calibrate_camera::Result>();
    result->success = true;
    result->camera_info = *camera_info;
    result->used_observations = static_cast<uint32_t>(observations.size());

    std::string save_error;
    if (!io::save_camera_info_yaml(result->camera_info, m_calibration_file,
                                   save_error)) {
        abort_active_session("Failed to save camera calibration: " + save_error);
        return;
    }

    std::ostringstream distortion_coefficients;
    distortion_coefficients << std::setprecision(17);
    for (size_t index = 0; index < result->camera_info.d.size(); ++index) {
        if (index != 0) {
            distortion_coefficients << ", ";
        }
        distortion_coefficients << result->camera_info.d[index];
    }
    RCLCPP_INFO(
        get_logger(),
        "Calibration calculated from %u observations and saved to '%s': "
        "image=%ux%u, distortion_model='%s', K=[%.17g, %.17g, %.17g; "
        "%.17g, %.17g, %.17g; %.17g, %.17g, %.17g], D=[%s]",
        result->used_observations, m_calibration_file.c_str(),
        result->camera_info.width, result->camera_info.height,
        result->camera_info.distortion_model.c_str(), result->camera_info.k[0],
        result->camera_info.k[1], result->camera_info.k[2],
        result->camera_info.k[3], result->camera_info.k[4],
        result->camera_info.k[5], result->camera_info.k[6],
        result->camera_info.k[7], result->camera_info.k[8],
        distortion_coefficients.str().c_str());

    if (m_active_goal != goal_handle || !m_calibrating) {
        return;
    }
    const auto set_camera_info_client = m_set_camera_info_client;
    cleanup_active_session();
    try {
        goal_handle->succeed(result);
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(),
                     "Skipping success result for an inactive goal: %s",
                     error.what());
        return;
    }

    if (!set_camera_info_client || !set_camera_info_client->service_is_ready()) {
        RCLCPP_WARN(
            get_logger(),
            "Calibration was saved to '%s', but camera set_camera_info service "
            "is not available; calculated parameters are available in the action "
            "result",
            m_calibration_file.c_str());
        return;
    }

    auto request = std::make_shared<set_camera_info::Request>();
    request->camera_info = result->camera_info;
    const auto logger = get_logger();
    try {
        set_camera_info_client->async_send_request(
            request, [logger, set_camera_info_client](
                         rclcpp::Client<set_camera_info>::SharedFuture future) {
                (void)set_camera_info_client;
                try {
                    const auto response = future.get();
                    if (!response->success) {
                        RCLCPP_WARN(
                            logger,
                            "Camera set_camera_info rejected saved calibration: %s",
                            response->status_message.empty()
                                ? "no reason provided"
                                : response->status_message.c_str());
                    }
                } catch (const std::exception& error) {
                    RCLCPP_WARN(logger,
                                "Camera set_camera_info failed after calibration "
                                "was saved: %s",
                                error.what());
                }
            });
    } catch (const std::runtime_error& error) {
        RCLCPP_WARN(logger,
                    "Could not send saved calibration to camera "
                    "set_camera_info service: %s",
                    error.what());
    }
}

bool camera_calibration::is_valid_goal(const calibrate_camera::Goal& goal,
                                       std::string& rejection_reason) const {
    if (goal.camera_namespace.empty() || goal.camera_namespace.front() != '/') {
        rejection_reason = "camera_namespace must be an absolute ROS namespace";
        return false;
    }
    if (!calibration::algorithm::factory::instance().contains(
            goal.calibration_method)) {
        rejection_reason =
            "unsupported calibration_method '" + goal.calibration_method + "'";
        return false;
    }
    if (!calibration::detector::factory::instance().contains(goal.board_type)) {
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

void camera_calibration::publish_feedback(
    const std::shared_ptr<goal_handle>& goal_handle, const bool board_detected,
    const uint8_t stage) const {
    if (m_active_goal != goal_handle) {
        return;
    }

    const auto progress = m_sample_selector.progress();
    const auto accepted_samples = m_sample_selector.accepted_count();

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

void camera_calibration::cancel_active_session(
    const std::shared_ptr<goal_handle>& goal_handle) {
    if (m_active_goal != goal_handle) {
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

void camera_calibration::abort_active_session(const std::string& reason) {
    if (!m_active_goal) {
        return;
    }
    const auto goal_handle = m_active_goal;
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

void camera_calibration::cleanup_active_session() {
    m_image_subscription.reset();
    m_debug_publisher.reset();
    m_set_camera_info_client.reset();
    m_detector.reset();
    m_sample_selector = calibration::sample::selector(m_sample_selector_config);
    m_calibrating = false;
    m_active_goal.reset();
    m_goal_reserved = false;
}

}  // namespace clover2_camera_utils

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_camera_utils::camera_calibration)
