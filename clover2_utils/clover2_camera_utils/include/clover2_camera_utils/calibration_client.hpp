#pragma once

// clover2
#include <clover2_camera_utils_msgs/action/calibrate_camera.hpp>
#include <clover2_camera_utils_msgs/srv/finish_calibration.hpp>

// ROS2
#include <rclcpp/create_client.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/create_client.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

// STL
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

namespace clover2_camera_utils {

class calibration_client {
public:
    using calibrate_camera = clover2_camera_utils_msgs::action::CalibrateCamera;
    using finish_calibration =
        clover2_camera_utils_msgs::srv::FinishCalibration;
    using action_client = rclcpp_action::Client<calibrate_camera>;
    using goal_handle = action_client::GoalHandle;
    using feedback_callback = action_client::FeedbackCallback;
    using result_callback = action_client::ResultCallback;
    using session_goal_id = std::optional<std::string>;

    class session {
    public:
        bool has_goal_id() const {
            return m_goal_id_state && m_goal_id_state->has_value();
        }

        const std::string& goal_id() const {
            if (!has_goal_id()) {
                throw std::runtime_error(
                    "Calibration goal ID is not available before first feedback");
            }
            return **m_goal_id_state;
        }

        rclcpp::Client<finish_calibration>::FutureAndRequestId finish() const {
            auto request = std::make_shared<finish_calibration::Request>();
            request->goal_id = goal_id();
            return m_finish_client->async_send_request(request);
        }

    private:
        friend class calibration_client;
        session(rclcpp::Client<finish_calibration>::SharedPtr finish_client,
                std::shared_ptr<session_goal_id> goal_id)
            : m_finish_client(std::move(finish_client)),
              m_goal_id_state(std::move(goal_id)) {}

        rclcpp::Client<finish_calibration>::SharedPtr m_finish_client;
        std::shared_ptr<session_goal_id> m_goal_id_state;
    };

    template <typename NodeT>
    explicit calibration_client(
        const NodeT& node, const std::string& base_path = "/camera_calibration",
        rclcpp::CallbackGroup::SharedPtr callback_group = nullptr)
        : m_action_client(rclcpp_action::create_client<calibrate_camera>(
              node, make_name(base_path, "calibrate"), callback_group))
        , m_finish_client(rclcpp::create_client<finish_calibration>(
              node->get_node_base_interface(), node->get_node_graph_interface(),
              node->get_node_services_interface(),
              make_name(base_path, "finish"), rclcpp::ServicesQoS(),
              callback_group)) {}

    session start(
        const calibrate_camera::Goal& goal,
        feedback_callback feedback_callback = nullptr,
        result_callback result_callback = nullptr) {
        auto goal_id = std::make_shared<session_goal_id>();
        m_sessions.emplace_back(goal_id);
        action_client::SendGoalOptions options;
        options.feedback_callback =
            [goal_id, feedback_callback = std::move(feedback_callback)](
                goal_handle::SharedPtr goal_handle,
                const std::shared_ptr<const calibrate_camera::Feedback>
                    feedback) {
                *goal_id = feedback->goal_id;
                if (feedback_callback) {
                    feedback_callback(std::move(goal_handle), feedback);
                }
            };
        options.result_callback =
            [this, goal_id, result_callback = std::move(result_callback)](
                const goal_handle::WrappedResult& result) {
                (void)result;
                std::erase_if(m_sessions, [&goal_id](const auto& weak_goal_id) {
                    const auto session_goal_id = weak_goal_id.lock();
                    return !session_goal_id || session_goal_id == goal_id;
                });
                if (result_callback) {
                    result_callback(result);
                }
            };

        m_action_client->async_send_goal(goal, options);
        return session(m_finish_client, std::move(goal_id));
    }

    std::vector<session> active_sessions() {
        std::vector<session> result;
        std::erase_if(m_sessions, [](const auto& goal_id) {
            return goal_id.expired();
        });
        for (const auto& weak_goal_id : m_sessions) {
            if (const auto goal_id = weak_goal_id.lock();
                goal_id && goal_id->has_value()) {
                result.push_back(session(m_finish_client, goal_id));
            }
        }
        return result;
    }

private:
    static std::string make_name(const std::string& base_path,
                                 const std::string& name) {
        if (base_path.empty() || base_path == "/") {
            return "/" + name;
        }
        return base_path + "/" + name;
    }

    action_client::SharedPtr m_action_client;
    rclcpp::Client<finish_calibration>::SharedPtr m_finish_client;
    std::vector<std::weak_ptr<session_goal_id>> m_sessions;
};

}  // namespace clover2_camera_utils