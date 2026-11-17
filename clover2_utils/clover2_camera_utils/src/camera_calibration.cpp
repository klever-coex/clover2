#include <clover2_camera_utils/camera_calibration.hpp>

#include <cv_bridge/cv_bridge.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/srv/set_camera_info.hpp>

#include <algorithm>
#include <exception>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace clover2_camera_utils {
namespace {

constexpr const char* k_image_topic_name = "image_raw";
constexpr const char* k_set_camera_info_service_name = "set_camera_info";

std::string format_calibration_description(
    const clover2_camera_utils_msgs::action::CalibrateCamera::Result& result) {
    std::ostringstream description;
    description << std::setprecision(17) << "Calibration calculated from "
                << result.used_observations
                << " observations: rms_reprojection_error="
                << result.rms_reprojection_error
                << " px, image=" << result.camera_info.width << "x"
                << result.camera_info.height << ", distortion_model='"
                << result.camera_info.distortion_model << "', K=[";
    for (size_t row = 0; row < 3; ++row) {
        if (row != 0) {
            description << "; ";
        }
        for (size_t column = 0; column < 3; ++column) {
            if (column != 0) {
                description << ", ";
            }
            description << result.camera_info.k[row * 3 + column];
        }
    }
    description << "], D=[";
    for (size_t index = 0; index < result.camera_info.d.size(); ++index) {
        if (index != 0) {
            description << ", ";
        }
        description << result.camera_info.d[index];
    }
    description << "]";
    return description.str();
}
}  // namespace

camera_calibration::camera_calibration(const rclcpp::NodeOptions& options)
    : clover2_common::node("camera_calibration", options)
    , m_sample_selector_config(*get_node_parameters_watcher_interface()) {
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

    RCLCPP_INFO(get_logger(), "Camera calibration Action server is ready");
}

camera_calibration::~camera_calibration() {
    m_sessions.clear();
    m_pending_targets.clear();
    m_action_server.reset();
}

rclcpp_action::GoalResponse camera_calibration::handle_goal(
    const rclcpp_action::GoalUUID& uuid,
    const std::shared_ptr<const calibrate_camera::Goal> goal) {
    std::string rejection_reason;
    if (!is_valid_goal(*goal, rejection_reason)) {
        RCLCPP_WARN(get_logger(), "Rejecting calibration goal: %s",
                    rejection_reason.c_str());
        return rclcpp_action::GoalResponse::REJECT;
    }

    const auto goal_id = rclcpp_action::to_string(uuid);
    try {
        m_pending_targets.emplace(
            goal_id, calibration::target::factory::instance().create(
                         goal->target_config_path));
    } catch (const std::exception& error) {
        RCLCPP_WARN(get_logger(), "Rejecting calibration goal: %s",
                    error.what());
        return rclcpp_action::GoalResponse::REJECT;
    }

    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse camera_calibration::handle_cancel(
    const std::shared_ptr<goal_handle> goal_handle) {
    const auto goal_id = rclcpp_action::to_string(goal_handle->get_goal_id());
    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end() ||
        session_it->second.action_goal_handle != goal_handle) {
        return rclcpp_action::CancelResponse::REJECT;
    }
    return rclcpp_action::CancelResponse::ACCEPT;
}

void camera_calibration::handle_accepted(
    const std::shared_ptr<goal_handle> goal_handle) {
    const auto goal_id = rclcpp_action::to_string(goal_handle->get_goal_id());
    const auto goal = goal_handle->get_goal();
    const auto image_topic = goal->camera_namespace + "/" + k_image_topic_name;
    const auto set_camera_info_service =
        goal->camera_namespace + "/" + k_set_camera_info_service_name;

    const auto pending_target_it = m_pending_targets.find(goal_id);
    if (pending_target_it == m_pending_targets.end()) {
        auto result = std::make_shared<calibrate_camera::Result>();
        result->success = false;
        result->message = "Missing prepared calibration target";
        goal_handle->abort(result);
        return;
    }
    auto target = std::move(pending_target_it->second);
    m_pending_targets.erase(pending_target_it);

    auto [session_it, unused] = m_sessions.emplace(
        std::piecewise_construct, std::forward_as_tuple(goal_id),
        std::forward_as_tuple(m_sample_selector_config));
    auto& calibration_session = session_it->second;
    calibration_session.action_goal_handle = goal_handle;
    calibration_session.target = std::move(target);
    calibration_session.algorithm =
        calibration::algorithm::factory::instance().create(
            goal->calibration_method);
    calibration_session.sample_selector.set_tilt_selection_enabled(false);
    calibration_session.debug_publisher =
        create_publisher<sensor_msgs::msg::Image>("~/output/debug",
                                                  rclcpp::SystemDefaultsQoS());
    calibration_session.set_camera_info_client = create_client<set_camera_info>(
        set_camera_info_service, rclcpp::ServicesQoS(),
        m_session_callback_group);

    rclcpp::SubscriptionOptions subscription_options;
    const auto image_qos = rclcpp::SensorDataQoS().keep_last(1);
    subscription_options.callback_group = m_session_callback_group;
    calibration_session.image_subscription =
        create_subscription<sensor_msgs::msg::Image>(
            image_topic, image_qos,
            [this,
             goal_id](const sensor_msgs::msg::Image::ConstSharedPtr message) {
                image_callback(goal_id, message);
            },
            subscription_options);

    RCLCPP_INFO(get_logger(),
                "Calibration session started for image topic '%s'; calculated "
                "parameters will be sent to '%s' when available",
                image_topic.c_str(), set_camera_info_service.c_str());
}

void camera_calibration::image_callback(
    const std::string& goal_id,
    const sensor_msgs::msg::Image::ConstSharedPtr message) {
    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end()) {
        return;
    }
    auto& calibration_session = session_it->second;
    if (calibration_session.calibrating) {
        return;
    }
    if (calibration_session.action_goal_handle->is_canceling()) {
        cancel_session(goal_id, calibration_session.action_goal_handle);
        return;
    }

    calibration_session::image_processing_result processing_result;
    try {
        const auto input =
            cv_bridge::toCvShare(message, sensor_msgs::image_encodings::MONO8);
        const bool publish_debug =
            calibration_session.debug_publisher &&
            calibration_session.debug_publisher->get_subscription_count() > 0;
        processing_result =
            calibration_session.process_image(input->image, publish_debug);

        if (publish_debug && processing_result.debug_image) {
            cv_bridge::CvImage output;
            output.header = message->header;
            output.encoding = sensor_msgs::image_encodings::BGR8;
            output.image = std::move(*processing_result.debug_image);
            calibration_session.debug_publisher->publish(*output.toImageMsg());
        }
        if (processing_result.initialization_event) {
            switch (*processing_result.initialization_event) {
                case calibration_session::calibration_initialization_event::
                    initial_estimate_failed:
                    RCLCPP_WARN(
                        get_logger(),
                        "Could not estimate camera parameters from %zu "
                        "selected "
                        "samples; collecting more diverse board poses",
                        calibration_session.sample_selector.accepted_count());
                    break;
                case calibration_session::calibration_initialization_event::
                    tilt_correction_failed:
                    RCLCPP_WARN(get_logger(),
                                "Could not calculate undistorted board tilt; "
                                "collecting "
                                "more diverse board poses");
                    break;
                case calibration_session::calibration_initialization_event::
                    tilt_selection_enabled:
                    RCLCPP_INFO(
                        get_logger(),
                        "Initial camera estimate succeeded after %zu selected "
                        "samples; enabling tilt selection",
                        calibration_session.sample_selector.accepted_count());
                    break;
            }
        }
    } catch (const std::exception& error) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "Failed to process calibration image: %s",
                             error.what());
    }

    publish_feedback(goal_id, processing_result.board_detected,
                     processing_result.feedback_stage);
}

void camera_calibration::handle_finish_calibration(
    const finish_calibration::Request::SharedPtr request,
    const finish_calibration::Response::SharedPtr response) {
    const auto& goal_id = request->goal_id;
    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end()) {
        response->accepted = false;
        response->message =
            "No calibration session found for the specified goal UUID";
        return;
    }
    auto& calibration_session = session_it->second;
    if (!start_calibration(goal_id, calibration_session, *response)) {
        return;
    }

    complete_calibration(goal_id, calibration_session.action_goal_handle,
                         calibration_session);
}

bool camera_calibration::start_calibration(
    const std::string& goal_id, calibration_session& calibration_session,
    finish_calibration::Response& response) {
    if (calibration_session.calibrating) {
        response.accepted = false;
        response.message = "Calibration is already running";
        return false;
    }
    if (!calibration_session.camera_estimate) {
        response.accepted = false;
        response.message = "Initial camera estimate is not ready";
        return false;
    }
    if (!calibration_session.sample_selector.progress().data_sufficient) {
        response.accepted = false;
        response.message = "Calibration dataset is not sufficient";
        return false;
    }

    calibration_session.calibrating = true;
    calibration_session.image_subscription.reset();
    calibration_session.debug_publisher.reset();

    response.accepted = true;
    response.message = "Calibration started";
    publish_feedback(goal_id, false, calibrate_camera::Feedback::CALIBRATING);
    return true;
}

void camera_calibration::complete_calibration(
    const std::string& goal_id, const std::shared_ptr<goal_handle>& goal_handle,
    calibration_session& calibration_session) {
    const auto calibration_result = calibration_session.run_calibration();

    if (!calibration_result) {
        abort_session(goal_id, "Camera calibration failed");
        return;
    }

    auto result = std::make_shared<calibrate_camera::Result>();
    result->success = true;
    result->camera_info = calibration_result->camera_info;
    result->rms_reprojection_error = calibration_result->rms_reprojection_error;
    result->used_observations = static_cast<uint32_t>(
        calibration_session.sample_selector.observations().size());

    RCLCPP_INFO(get_logger(), "%s",
                format_calibration_description(*result).c_str());

    const auto set_camera_info_client =
        calibration_session.set_camera_info_client;
    cleanup_session(goal_id);
    try {
        goal_handle->succeed(result);
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(),
                     "Skipping success result for an inactive goal: %s",
                     error.what());
        return;
    }

    send_camera_info(set_camera_info_client, result->camera_info);
}

void camera_calibration::send_camera_info(
    const rclcpp::Client<set_camera_info>::SharedPtr& client,
    const sensor_msgs::msg::CameraInfo& camera_info) const {
    if (!client || !client->service_is_ready()) {
        RCLCPP_WARN(
            get_logger(),
            "Camera set_camera_info service is not available; calculated "
            "parameters are available in the action result");
        return;
    }

    auto set_camera_info_request = std::make_shared<set_camera_info::Request>();
    set_camera_info_request->camera_info = camera_info;
    const auto logger = get_logger();
    try {
        client->async_send_request(
            set_camera_info_request,
            [logger,
             client](rclcpp::Client<set_camera_info>::SharedFuture future) {
                (void)client;
                try {
                    const auto response = future.get();
                    if (!response->success) {
                        RCLCPP_WARN(
                            logger,
                            "Camera set_camera_info rejected calculated "
                            "calibration: %s",
                            response->status_message.empty()
                                ? "no reason provided"
                                : response->status_message.c_str());
                    }
                } catch (const std::exception& error) {
                    RCLCPP_WARN(logger,
                                "Camera set_camera_info failed for calculated "
                                "calibration: %s",
                                error.what());
                }
            });
    } catch (const std::runtime_error& error) {
        RCLCPP_WARN(logger,
                    "Could not send calculated calibration to camera "
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
    if (goal.target_config_path.empty()) {
        rejection_reason = "target_config_path must not be empty";
        return false;
    }
    return true;
}

void camera_calibration::publish_feedback(const std::string& goal_id,
                                          const bool board_detected,
                                          const uint8_t stage) const {
    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end()) {
        return;
    }
    const auto& calibration_session = session_it->second;

    const auto progress = calibration_session.sample_selector.progress();
    const auto accepted_samples =
        calibration_session.sample_selector.accepted_count();

    auto feedback = std::make_shared<calibrate_camera::Feedback>();
    feedback->goal_id = goal_id;
    feedback->accepted_samples = static_cast<uint32_t>(accepted_samples);
    feedback->progress_x = static_cast<float>(progress.x);
    feedback->progress_y = static_cast<float>(progress.y);
    feedback->progress_size = static_cast<float>(progress.size);
    feedback->progress_tilt_x = static_cast<float>(progress.tilt_x);
    feedback->progress_tilt_y = static_cast<float>(progress.tilt_y);
    feedback->data_sufficient = progress.data_sufficient &&
                                calibration_session.camera_estimate.has_value();
    feedback->stage = stage;
    if (stage == calibrate_camera::Feedback::SAMPLE_ACCEPTED) {
        feedback->hint = "Calibration sample accepted.";
    } else if (stage == calibrate_camera::Feedback::DATA_SUFFICIENT) {
        feedback->hint = "Calibration dataset is sufficient.";
    } else if (stage == calibrate_camera::Feedback::CALIBRATING) {
        feedback->hint = "Computing camera calibration.";
    } else {
        feedback->hint =
            board_detected
                ? "Calibration board detected. Evaluating the sample."
                : "Show the configured calibration board to the camera.";
    }
    try {
        calibration_session.action_goal_handle->publish_feedback(feedback);
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(), "Skipping feedback for an inactive goal: %s",
                     error.what());
    }
}

void camera_calibration::cancel_session(
    const std::string& goal_id,
    const std::shared_ptr<goal_handle>& goal_handle) {
    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end() ||
        session_it->second.action_goal_handle != goal_handle) {
        return;
    }
    cleanup_session(goal_id);

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

void camera_calibration::abort_session(const std::string& goal_id,
                                       const std::string& reason) {
    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end()) {
        return;
    }
    const auto goal_handle = session_it->second.action_goal_handle;
    cleanup_session(goal_id);

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

void camera_calibration::cleanup_session(const std::string& goal_id) {
    m_sessions.erase(goal_id);
}

}  // namespace clover2_camera_utils

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_camera_utils::camera_calibration)
