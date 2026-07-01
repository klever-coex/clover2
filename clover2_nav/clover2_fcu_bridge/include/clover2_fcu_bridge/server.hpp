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
#include <bondcpp/bond.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/timer.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

// STL
#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
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

    /// @brief Checks if a new goal can be accepted and reserves a slot 
    //  before calling the accepted callback
    /// @details Offboard navigation starts only after the bond is formed.
    rclcpp_action::GoalResponse handle_navigate_async_goal(
        const rclcpp_action::GoalUUID& uuid,
        std::shared_ptr<const NavigateAsync::Goal> goal);
    /// @brief Accepts cancellation only for the current navigation action.
    rclcpp_action::CancelResponse handle_navigate_async_cancel(
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);
    /// @brief Callback saves the accepted goal and launches the associated bond.
    void handle_navigate_async_accepted(
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);

    /// @brief Publishes feedback and finishes the action after navigation starts.
    void process_navigate_async(
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);

    void extract_target_pose(const geometry_msgs::msg::Pose& pose,
                             std::optional<double>& x, std::optional<double>& y,
                             std::optional<double>& z,
                             std::optional<double>& yaw) const;

    /// @brief Creates a watchdog bond with the unique action goal UUID.
    void start_navigate_bond(
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);
    /// @brief Closes the current bond normally without emergency handling.
    void cleanup_navigate_bond();
    /// @brief Starts navigation after the client bond is formed.
    void handle_navigate_bond_formed(
        const std::string& bond_id,
        const std::shared_ptr<GoalHandleNavigateAsync> goal_handle);
    /// @brief Stops navigation and lands the vehicle when the action is lost.
    void handle_navigate_bond_broken(const std::string& bond_id);

    double m_speed_limit{1.0};
    double m_tolerance{0.15};
    double m_slowdown{0.5};

    std::string m_backend_name;
    clover2_common::parameter_watcher::SharedPtr m_parameter_watcher;

    rclcpp::CallbackGroup::SharedPtr m_service_callback_group;

    /// @brief Bond that watches the lifetime of the current navigation action.
    std::shared_ptr<bond::Bond> m_navigate_bond;

    /// @brief Bond id built from the current goal UUID.
    std::string m_navigate_bond_id;

    /// @brief Indicates that the server is closing the bond normally.
    /// @details `true` means the server is performing an expected `breakBond()`;
    /// `false` means the bond is active or was broken unexpectedly.
    bool m_navigate_bond_closing{false};

    rclcpp::TimerBase::SharedPtr m_state_publish_timer;

    rclcpp::Publisher<clover2_nav_msgs::msg::State>::SharedPtr m_state_pub;

    rclcpp::Service<clover2_nav_msgs::srv::ArmDisarm>::SharedPtr
        m_arm_disarm_srv;
    rclcpp::Service<clover2_nav_msgs::srv::Land>::SharedPtr m_land_srv;
    rclcpp::Service<clover2_nav_msgs::srv::SetPosition>::SharedPtr
        m_set_position_srv;
    rclcpp::Service<clover2_nav_msgs::srv::Navigate>::SharedPtr m_navigate_srv;

    rclcpp_action::Server<NavigateAsync>::SharedPtr m_navigate_async_action;
    /// @brief Goal that owns the current navigation and operation bond.
    std::shared_ptr<GoalHandleNavigateAsync> m_active_navigate_goal;

    /// @brief Time when the current navigation entered idle before normal success.
    std::optional<std::chrono::steady_clock::time_point>
        m_navigate_completion_started_at;

    /// @brief Indicates that a new goal is waiting for the accepted callback.
    /// @details `true` means the goal callback accepted the goal but the accepted
    /// callback has not run yet; `false` means there is no pending goal.
    std::atomic_bool m_navigate_goal_pending{false};

    std::shared_ptr<clover2_fcu_bridge::backend::base_backend> m_backend{nullptr};
    std::shared_ptr<clover2_fcu_bridge::offboard> m_offboard{nullptr};
};

}  // namespace clover2_fcu_bridge
