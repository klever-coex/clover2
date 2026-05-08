#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_fcu_bridge/backend/fabric.hpp>

// ROS2
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <tf2/LinearMath/Vector3.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_broadcaster.hpp>
#include <tf2_ros/transform_listener.h>

// STL
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace clover2_fcu_bridge {

class offboard {
public:
    using process_callback = std::function<void()>;

    enum state {  //
        idle,
        position,
        navigation,
        slow_takeoff
    };

    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(offboard)

    static std::vector<std::string> list_backends();

    template <typename NodeT>
    explicit offboard(
        const std::shared_ptr<NodeT> node,
        std::weak_ptr<clover2_fcu_bridge::backend::base_backend> backend,
        rclcpp::CallbackGroup::SharedPtr group = nullptr)
        : m_backend(backend)
        , m_logger(node->get_logger().get_child("offboard"))
        , m_clock(node->get_clock())
        , m_group(group) {
        m_tf_buffer = std::make_shared<tf2_ros::Buffer>(node->get_clock());
        m_tf_listener =
            std::make_shared<tf2_ros::TransformListener>(*m_tf_buffer, node);

        if (m_group.get() == nullptr) {
            m_group = node->create_callback_group(
                rclcpp::CallbackGroupType::MutuallyExclusive);
        }

        m_update_timer = node->create_wall_timer(
            m_publish_period,
            [this]() -> void {  //
                publish_offboard();
            },
            m_group);

        RCLCPP_INFO(get_logger(), "Offboard initialized");
    }

    virtual ~offboard();

    bool in_idle() const;
    void reset_state();
    void set_process_callback(process_callback&& cb);

    bool is_armed() const { return m_backend.lock()->is_armed(); }
    void nav_current_diff(tf2::Vector3& diff_pos, double& diff_yaw) const;

    void set_position(const std::string& frame_id, std::optional<double> x,
                      std::optional<double> y, std::optional<double> z,
                      std::optional<double> yaw);
    void navigate(const std::string& frame_id, std::optional<double> x,
                  std::optional<double> y, std::optional<double> z,
                  std::optional<double> yaw, double speed = 0.3);

    void get_position(double& x, double& y, double& z, double& yaw) const;
    geometry_msgs::msg::PoseStamped get_position() const;

    void arm() { return m_backend.lock()->arm(); }
    void disarm() { return m_backend.lock()->disarm(); }

    void set_tolerance(double tolerance = 0.05) { m_tolerance = tolerance; }
    double get_tolerance() const { return m_tolerance; }
    void set_slowdown_distance(double distance = 0.3) {
        m_slowdown_distance = distance;
    }
    double get_slowdown_distance() const { return m_slowdown_distance; }

private:
    void extract_pose(const geometry_msgs::msg::PoseStamped& pose, double& x,
                      double& y, double& z, double& yaw) const;
    void extract_pose(const geometry_msgs::msg::PoseStamped& pose,
                      tf2::Vector3& pos, double& yaw) const;

    void complite_setpoint(const std::string& frame_id,
                           const std::optional<double>& x,
                           const std::optional<double>& y,
                           const std::optional<double>& z,
                           const std::optional<double>& yaw,
                           const geometry_msgs::msg::PoseStamped& ref_pose,
                           geometry_msgs::msg::PoseStamped& pose) const;

    void compute_diff(const geometry_msgs::msg::PoseStamped& ref,
                      tf2::Vector3& diff_pos, double& diff_yaw) const;

    void set_yaw(geometry_msgs::msg::Quaternion& q, double yaw) const;

    void check_fcu(state& process_state);

    void publish_offboard();

    void publish_position();
    void publish_velocity();
    void update_navigation_setpoint();
    void check_action_complite(
        const geometry_msgs::msg::PoseStamped& target_pose);

    void change_state(const state new_state);

    rclcpp::Logger get_logger() const { return m_logger; }
    rclcpp::Clock::SharedPtr get_clock() const { return m_clock; }

    clover2_fcu_bridge::backend::base_backend::WeakPtr m_backend;

    bool m_reset_require{true};
    state m_state{state::idle};
    std::string m_local_frame{"map"};
    std::string m_setpoint_frame{"setpoint"};
    process_callback m_process_callback{nullptr};

    bool m_check_fcu{false};
    double m_speed{0.3};
    double m_speed_limit{2.0};
    double m_yaw_speed{0.1};
    double m_height_low{0.3};
    double m_tolerance{0.05};
    double m_slowdown_distance{0.3};
    std::chrono::milliseconds m_publish_period{20};
    geometry_msgs::msg::PoseStamped m_nav_setpoint, m_pose_setpoint,
        m_current_pose;
    geometry_msgs::msg::Twist m_velocity_setpoint;

    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    std::shared_ptr<tf2_ros::Buffer> m_tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> m_tf_listener;
    rclcpp::CallbackGroup::SharedPtr m_group;
    rclcpp::TimerBase::SharedPtr m_update_timer;
};

}  // namespace clover2_fcu_bridge
