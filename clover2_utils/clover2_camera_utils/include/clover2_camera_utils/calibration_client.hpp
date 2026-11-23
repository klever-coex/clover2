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
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

namespace clover2_camera_utils {

class calibration_client {
public:
    using calibrate_camera = clover2_camera_utils_msgs::action::CalibrateCamera;
    using finish_calibration =
        clover2_camera_utils_msgs::srv::FinishCalibration;
    using action_client = rclcpp_action::Client<calibrate_camera>;
    using goal_handle = action_client::GoalHandle;
    using feedback_type = calibrate_camera::Feedback;
    using goal_response_callback = std::function<void(goal_handle::SharedPtr)>;
    using feedback_callback = action_client::FeedbackCallback;
    using result_callback = action_client::ResultCallback;
    class session {
    public:
        std::shared_ptr<const feedback_type> snapshot() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_feedback;
        }

        bool has_goal_id() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return !m_goal_id.empty();
        }

        std::string goal_id() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_goal_id.empty()) {
                throw std::runtime_error(
                    "Calibration goal ID is not available before goal acceptance");
            }
            return m_goal_id;
        }

        rclcpp::Client<finish_calibration>::FutureAndRequestId finish() const {
            auto request = std::make_shared<finish_calibration::Request>();
            request->goal_id = goal_id();
            return m_finish_client->async_send_request(request);
        }

    private:
        friend class calibration_client;
        explicit session(
            rclcpp::Client<finish_calibration>::SharedPtr finish_client)
            : m_finish_client(std::move(finish_client)) {}

        void set_feedback(std::shared_ptr<const feedback_type> feedback) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_feedback = std::move(feedback);
        }

        void set_goal_id(std::string goal_id) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_goal_id = std::move(goal_id);
        }

        rclcpp::Client<finish_calibration>::SharedPtr m_finish_client;
        mutable std::mutex m_mutex;
        std::string m_goal_id;
        std::shared_ptr<const feedback_type> m_feedback;
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

    std::shared_ptr<session> start(
        const calibrate_camera::Goal& goal,
        goal_response_callback goal_response_callback = nullptr,
        feedback_callback feedback_callback = nullptr,
        result_callback result_callback = nullptr) {
        auto calibration_session = std::shared_ptr<session>(
            new session(m_finish_client));
        {
            std::lock_guard<std::mutex> lock(m_sessions_mutex);
            m_sessions.push_back(calibration_session);
        }

        action_client::SendGoalOptions options;
        options.goal_response_callback =
            [this, calibration_session,
             goal_response_callback = std::move(goal_response_callback)](
                goal_handle::SharedPtr handle) {
                if (!handle) {
                    remove_session(calibration_session);
                } else {
                    calibration_session->set_goal_id(
                        rclcpp_action::to_string(handle->get_goal_id()));
                }
                if (goal_response_callback) {
                    goal_response_callback(std::move(handle));
                }
            };
        options.feedback_callback =
            [calibration_session,
             feedback_callback = std::move(feedback_callback)](
                goal_handle::SharedPtr goal_handle,
                const std::shared_ptr<const calibrate_camera::Feedback>
                    feedback) {
                calibration_session->set_feedback(feedback);
                if (feedback_callback) {
                    feedback_callback(std::move(goal_handle), feedback);
                }
            };
        options.result_callback =
            [this, calibration_session,
             result_callback = std::move(result_callback)](
                const goal_handle::WrappedResult& result) {
                remove_session(calibration_session);
                if (result_callback) {
                    result_callback(result);
                }
            };

        try {
            m_action_client->async_send_goal(goal, options);
        } catch (...) {
            remove_session(calibration_session);
            throw;
        }
        return calibration_session;
    }

    std::list<std::shared_ptr<session>> active_sessions() const {
        std::lock_guard<std::mutex> lock(m_sessions_mutex);
        return m_sessions;
    }

    std::shared_ptr<session> find_session(const std::string& goal_id) const {
        std::lock_guard<std::mutex> lock(m_sessions_mutex);
        for (const auto& active_session : m_sessions) {
            if (active_session->has_goal_id() &&
                active_session->goal_id() == goal_id) {
                return active_session;
            }
        }
        return nullptr;
    }

private:
    void remove_session(const std::shared_ptr<session>& target) {
        std::lock_guard<std::mutex> lock(m_sessions_mutex);
        m_sessions.remove(target);
    }

    static std::string make_name(const std::string& base_path,
                                 const std::string& name) {
        if (base_path.empty() || base_path == "/") {
            return "/" + name;
        }
        return base_path + "/" + name;
    }

    action_client::SharedPtr m_action_client;
    rclcpp::Client<finish_calibration>::SharedPtr m_finish_client;
    mutable std::mutex m_sessions_mutex;
    std::list<std::shared_ptr<session>> m_sessions;
};

}  // namespace clover2_camera_utils