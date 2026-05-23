#pragma once

// clover2
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/parameter_watcher.hpp>
#include <clover2_fcu_bridge/backend/base_backend.hpp>
#include <clover2_fcu_bridge/offboard.hpp>
#include <clover2_nav_msgs/action/navigate_async.hpp>
#include <clover2_nav_msgs/msg/state.hpp>
#include <clover2_nav_msgs/srv/arm_disarm.hpp>
#include <clover2_nav_msgs/srv/land.hpp>
#include <clover2_nav_msgs/srv/navigate.hpp>
#include <clover2_nav_msgs/srv/set_position.hpp>

// ROS2
#include <rclcpp/publisher.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/timer.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_fcu_bridge {

using NavigateAsync = clover2_nav_msgs::action::NavigateAsync;
using GoalHandleNavigateAsync = rclcpp_action::ServerGoalHandle<NavigateAsync>;

class server : public clover2_common::lifecycle_node {
public:
    using SharedPtr = std::shared_ptr<server>;
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::
        LifecycleNodeInterface::CallbackReturn;

    explicit server(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    CallbackReturn on_configure(const rclcpp_lifecycle::State& state);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state);

private:
    void handle_arm_disarm(
        const clover2_nav_msgs::srv::ArmDisarm::Request::SharedPtr req,
        clover2_nav_msgs::srv::ArmDisarm::Response::SharedPtr resp);

    void handle_land(
        const clover2_nav_msgs::srv::Land::Request::SharedPtr req,
        clover2_nav_msgs::srv::Land::Response::SharedPtr resp);

    void handle_set_position(
        const clover2_nav_msgs::srv::SetPosition::Request::SharedPtr req,
        clover2_nav_msgs::srv::SetPosition::Response::SharedPtr resp);

    void handle_navigate(
        const clover2_nav_msgs::srv::Navigate::Request::SharedPtr req,
        clover2_nav_msgs::srv::Navigate::Response::SharedPtr resp);

    rclcpp_action::GoalResponse handle_navigate_async_goal(
        const rclcpp_action::GoalUUID& uuid,
        std::shared_ptr<const NavigateAsync::Goal> goal);
    rclcpp_action::CancelResponse handle_navigate_async_cancel(
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);
    void handle_navigate_async_accepted(
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);

    void process_navigate_async(
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);

    void extract_target_pose(const geometry_msgs::msg::Pose& pose,
                             std::optional<double>& x, std::optional<double>& y,
                             std::optional<double>& z,
                             std::optional<double>& yaw) const;

    double m_speed_limit{1.0};
    double m_tolerance{0.15};
    double m_slowdown{0.5};

    std::string m_backend_name;
    clover2_common::parameter_watcher::SharedPtr m_parameter_watcher;

    rclcpp::CallbackGroup::SharedPtr m_service_callback_group;

    rclcpp::TimerBase::SharedPtr m_state_publish_timer;

    rclcpp::Publisher<clover2_nav_msgs::msg::State>::SharedPtr m_state_pub;

    rclcpp::Service<clover2_nav_msgs::srv::ArmDisarm>::SharedPtr
        m_arm_disarm_srv;
    rclcpp::Service<clover2_nav_msgs::srv::Land>::SharedPtr m_land_srv;
    rclcpp::Service<clover2_nav_msgs::srv::SetPosition>::SharedPtr
        m_set_position_srv;
    rclcpp::Service<clover2_nav_msgs::srv::Navigate>::SharedPtr m_navigate_srv;

    rclcpp_action::Server<NavigateAsync>::SharedPtr m_navigate_async_action;

    std::shared_ptr<clover2_fcu_bridge::backend::base_backend> m_backend{nullptr};
    std::shared_ptr<clover2_fcu_bridge::offboard> m_offboard{nullptr};
};

}  // namespace clover2_fcu_bridge
