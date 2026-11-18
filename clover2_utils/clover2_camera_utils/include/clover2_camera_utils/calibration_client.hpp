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

    std::shared_future<goal_handle::SharedPtr> start(
        const calibrate_camera::Goal& goal,
        feedback_callback feedback_callback = nullptr,
        result_callback result_callback = nullptr) {
        m_goal_id.reset();

        action_client::SendGoalOptions options;
        options.feedback_callback =
            [this, feedback_callback = std::move(feedback_callback)](
                goal_handle::SharedPtr goal_handle,
                const std::shared_ptr<const calibrate_camera::Feedback>
                    feedback) {
                m_goal_id = feedback->goal_id;
                if (feedback_callback) {
                    feedback_callback(std::move(goal_handle), feedback);
                }
            };
        options.result_callback =
            [this, result_callback = std::move(result_callback)](
                const goal_handle::WrappedResult& result) {
                m_goal_id.reset();
                if (result_callback) {
                    result_callback(result);
                }
            };

        return m_action_client->async_send_goal(goal, options);
    }

    rclcpp::Client<finish_calibration>::FutureAndRequestId finish() {
        if (!m_goal_id) {
            throw std::runtime_error(
                "Cannot finish calibration before receiving action feedback");
        }

        auto request = std::make_shared<finish_calibration::Request>();
        request->goal_id = *m_goal_id;
        return m_finish_client->async_send_request(request);
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
    std::optional<std::string> m_goal_id;
};

}  // namespace clover2_camera_utils
