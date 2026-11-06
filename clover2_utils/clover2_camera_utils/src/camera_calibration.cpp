#include <clover2_camera_utils/camera_calibration.hpp>

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/srv/set_camera_info.hpp>

#include <algorithm>
#include <exception>
#include <filesystem>
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
const std::filesystem::path k_detected_frames_directory =
    "/tmp/clover2_camera_calibration_detections";

}  // namespace

camera_calibration::camera_calibration(const rclcpp::NodeOptions& options)
    : clover2_common::node("camera_calibration", options) {
    const auto declare_progress_parameter = [this](const char* name,
                                                   size_t index) {
        declare_and_watch_parameter<double>(
            name, m_sample_selector_config.minimum_progress[index],
            [this, index, name](const rclcpp::Parameter& parameter) {
                const auto minimum_progress = parameter.as_double();
                if (minimum_progress < 0.0 || minimum_progress > 1.0) {
                    throw std::invalid_argument(std::string(name) +
                                                " must be in [0.0, 1.0]");
                }
                m_sample_selector_config.minimum_progress[index] =
                    minimum_progress;
            });
    };
    declare_progress_parameter("minimum_progress_x", 0);
    declare_progress_parameter("minimum_progress_y", 1);
    declare_progress_parameter("minimum_progress_size", 2);
    declare_progress_parameter("minimum_progress_skew", 3);

    declare_and_watch_parameter<int>(
        "minimum_samples_for_completion",
        static_cast<int>(
            m_sample_selector_config.minimum_samples_for_completion),
        [this](const rclcpp::Parameter& parameter) {
            const auto minimum_samples = parameter.as_int();
            if (minimum_samples < 3) {
                throw std::invalid_argument(
                    "minimum_samples_for_completion must be at least 3");
            }
            m_sample_selector_config.minimum_samples_for_completion =
                static_cast<size_t>(minimum_samples);
        });

    for (const auto [name, index] :
         {std::pair{"minimum_progress_x", size_t{0}},
          std::pair{"minimum_progress_y", size_t{1}},
          std::pair{"minimum_progress_size", size_t{2}},
          std::pair{"minimum_progress_skew", size_t{3}}}) {
        const auto minimum_progress = get_parameter(name).as_double();
        if (minimum_progress < 0.0 || minimum_progress > 1.0) {
            throw std::invalid_argument(std::string(name) +
                                        " must be in [0.0, 1.0]");
        }
        m_sample_selector_config.minimum_progress[index] = minimum_progress;
    }

    const auto minimum_samples =
        get_parameter("minimum_samples_for_completion").as_int();
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
    auto topic_prefix = goal->camera_namespace;
    if (topic_prefix != "/") {
        topic_prefix += "/";
    }
    const auto image_topic = topic_prefix + k_image_topic_name;
    const auto set_camera_info_service =
        topic_prefix + k_set_camera_info_service_name;

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

    auto [session_it, inserted] = m_sessions.emplace(
        std::piecewise_construct, std::forward_as_tuple(goal_id),
        std::forward_as_tuple(m_sample_selector_config));
    if (!inserted) {
        auto result = std::make_shared<calibrate_camera::Result>();
        result->success = false;
        result->message = "Duplicate calibration goal UUID";
        goal_handle->abort(result);
        return;
    }
    auto& calibration_session = session_it->second;
    calibration_session.action_goal_handle = goal_handle;
    calibration_session.target = std::move(target);

    const auto image_qos = rclcpp::SensorDataQoS().keep_last(1);
    calibration_session.debug_publisher =
        create_publisher<sensor_msgs::msg::Image>("~/output/debug",
                                                  rclcpp::SystemDefaultsQoS());
    calibration_session.set_camera_info_client = create_client<set_camera_info>(
        set_camera_info_service, rclcpp::ServicesQoS(),
        m_session_callback_group);
    rclcpp::SubscriptionOptions subscription_options;
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
    bool board_detected = false;
    uint8_t stage = calibrate_camera::Feedback::WAITING_FOR_BOARD;
    std::optional<data::target_detection> sample;
    try {
        const auto input =
            cv_bridge::toCvShare(message, sensor_msgs::image_encodings::MONO8);
        sample = calibration_session.target->detect(input->image);
        board_detected = sample.has_value();
        stage = board_detected ? calibrate_camera::Feedback::BOARD_DETECTED
                               : calibrate_camera::Feedback::WAITING_FOR_BOARD;

        cv::Mat debug_image;
        const bool publish_debug =
            calibration_session.debug_publisher &&
            calibration_session.debug_publisher->get_subscription_count() > 0;
        const bool prepare_debug_image = publish_debug || sample.has_value();
        if (prepare_debug_image) {
            cv::cvtColor(input->image, debug_image, cv::COLOR_GRAY2BGR);
            calibration_session.target->draw_debug(debug_image, sample);
        }

        if (sample) {
            const auto selection =
                calibration_session.sample_selector.try_add(std::move(*sample));
            if (selection.progress.data_sufficient) {
                stage = calibrate_camera::Feedback::DATA_SUFFICIENT;
            } else if (selection.accepted) {
                stage = calibrate_camera::Feedback::SAMPLE_ACCEPTED;
            }

            if (selection.accepted) {
                const auto accepted_frames =
                    calibration_session.sample_selector.accepted_count();
                const auto session_directory =
                    k_detected_frames_directory / goal_id;
                std::filesystem::create_directories(session_directory);
                std::ostringstream filename;
                filename << "accepted_" << std::setw(6) << std::setfill('0')
                         << accepted_frames << ".png";
                const auto output_path = session_directory / filename.str();
                if (!cv::imwrite(output_path.string(), debug_image)) {
                    RCLCPP_WARN(get_logger(),
                                "Failed to save accepted calibration frame to '%s'",
                                output_path.c_str());
                }
            }
        }

        if (publish_debug) {
            cv::putText(
                debug_image,
                "Accepted frames: " + std::to_string(
                                         calibration_session.sample_selector
                                             .accepted_count()),
                cv::Point{10, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar{0, 255, 0}, 2, cv::LINE_AA);

            cv_bridge::CvImage output;
            output.header = message->header;
            output.encoding = sensor_msgs::image_encodings::BGR8;
            output.image = std::move(debug_image);
            calibration_session.debug_publisher->publish(*output.toImageMsg());
        }
    } catch (const std::exception& error) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "Failed to process calibration image: %s",
                             error.what());
    }

    publish_feedback(goal_id, board_detected, stage);
}

void camera_calibration::handle_finish_calibration(
    const finish_calibration::Request::SharedPtr request,
    const finish_calibration::Response::SharedPtr response) {
    const auto& goal_id = request->goal_id;
    std::shared_ptr<goal_handle> goal_handle;
    std::vector<data::observation> observations;
    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end()) {
        response->accepted = false;
        response->message =
            "No calibration session found for the specified goal UUID";
        return;
    }
    auto& calibration_session = session_it->second;
    if (calibration_session.calibrating) {
        response->accepted = false;
        response->message = "Calibration is already running";
        return;
    }
    if (!calibration_session.sample_selector.progress().data_sufficient) {
        response->accepted = false;
        response->message = "Calibration dataset is not sufficient";
        return;
    }

    goal_handle = calibration_session.action_goal_handle;
    observations = calibration_session.sample_selector.observations();
    calibration_session.calibrating = true;
    calibration_session.image_subscription.reset();
    calibration_session.debug_publisher.reset();

    response->accepted = true;
    response->message = "Calibration started";
    publish_feedback(goal_id, false, calibrate_camera::Feedback::CALIBRATING);
    run_calibration(goal_id, goal_handle, std::move(observations));
}

void camera_calibration::run_calibration(
    const std::string& goal_id, std::shared_ptr<goal_handle> goal_handle,
    std::vector<data::observation> observations) {
    const auto algorithm = calibration::algorithm::factory::instance().create(
        goal_handle->get_goal()->calibration_method);
    const auto calibration_result = algorithm->calibrate(observations);

    if (!calibration_result) {
        abort_session(goal_id, "Camera calibration failed");
        return;
    }

    auto result = std::make_shared<calibrate_camera::Result>();
    result->success = true;
    result->camera_info = calibration_result->camera_info;
    result->rms_reprojection_error = calibration_result->rms_reprojection_error;
    result->used_observations = static_cast<uint32_t>(observations.size());

    std::ostringstream calibration_description;
    calibration_description
        << std::setprecision(17) << "Calibration calculated from "
        << result->used_observations
        << " observations: rms_reprojection_error="
        << result->rms_reprojection_error << " px, image="
        << result->camera_info.width << "x"
        << result->camera_info.height << ", distortion_model='"
        << result->camera_info.distortion_model << "', K=[";
    for (size_t row = 0; row < 3; ++row) {
        if (row != 0) {
            calibration_description << "; ";
        }
        for (size_t column = 0; column < 3; ++column) {
            if (column != 0) {
                calibration_description << ", ";
            }
            calibration_description << result->camera_info.k[row * 3 + column];
        }
    }
    calibration_description << "], D=[";
    for (size_t index = 0; index < result->camera_info.d.size(); ++index) {
        if (index != 0) {
            calibration_description << ", ";
        }
        calibration_description << result->camera_info.d[index];
    }
    calibration_description << "]";
    RCLCPP_INFO(get_logger(), "%s", calibration_description.str().c_str());

    const auto session_it = m_sessions.find(goal_id);
    if (session_it == m_sessions.end() ||
        session_it->second.action_goal_handle != goal_handle ||
        !session_it->second.calibrating) {
        return;
    }
    const auto set_camera_info_client =
        session_it->second.set_camera_info_client;
    cleanup_session(goal_id);
    try {
        goal_handle->succeed(result);
    } catch (const std::runtime_error& error) {
        RCLCPP_DEBUG(get_logger(),
                     "Skipping success result for an inactive goal: %s",
                     error.what());
        return;
    }

    if (!set_camera_info_client ||
        !set_camera_info_client->service_is_ready()) {
        RCLCPP_WARN(
            get_logger(),
            "Camera set_camera_info service is not available; calculated "
            "parameters are available in the action result");
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
