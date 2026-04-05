#pragma once

// clover2
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <clover2/common/node_context.hpp>
#include <clover2_offboard/backend/fabric.hpp>

// ROS2
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <tf2/LinearMath/Vector3.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

// STL
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace clover2_offboard {

class helper {
public:
  enum state { none, position, navigation };

  RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(helper)

  static std::vector<std::string> list_backends();

  template <typename NodeT>
  explicit helper(const std::shared_ptr<NodeT> node, const std::string &backend,
                  rclcpp::CallbackGroup::SharedPtr group = nullptr)
      : m_logger(node->get_logger().get_child("helper")),
        m_clock(node->get_clock()), m_group(group) {
    backend::context ctx(*node);
    m_backend = backend::fabric::instance().create(backend, ctx);

    m_tf_buffer = std::make_shared<tf2_ros::Buffer>(node->get_clock());
    m_tf_listener =
        std::make_shared<tf2_ros::TransformListener>(*m_tf_buffer, node);

    if (m_group.get() == nullptr) {
      m_group = node->create_callback_group(
          rclcpp::CallbackGroupType::MutuallyExclusive);
    }

    m_update_timer = node->create_wall_timer(
        m_publish_period,
        [this]() -> void { //
          publish_offboard();
        },
        m_group);

    RCLCPP_INFO(get_logger(), "Backend initialized: %s", backend.c_str());
  }

  virtual ~helper();

  void set_position(const std::string &frame_id, std::optional<double> x,
                    std::optional<double> y, std::optional<double> z,
                    std::optional<double> yaw);

  void navigate(const std::string &frame_id, std::optional<double> x,
                std::optional<double> y, std::optional<double> z,
                std::optional<double> yaw, double speed = 0.3);

  void get_position(double &x, double &y, double &z, double &yaw) const;

  bool is_armed() const { return m_backend->is_armed(); }
  void arm() { return m_backend->arm(); }
  void disarm() { return m_backend->disarm(); }

  void set_tolerance(double tolerance = 0.05) { m_tolerance = tolerance; }
  double get_tolerance() const { return m_tolerance; }

  void set_slowdown_distance(double distance = 0.3) {
    m_slowdown_distance = distance;
  }
  double get_slowdown_distance() const { return m_slowdown_distance; }

private:
  void extract_pose(const geometry_msgs::msg::PoseStamped &pose, double &x,
                    double &y, double &z, double &yaw) const;
  void extract_pose(const geometry_msgs::msg::PoseStamped &pose,
                    tf2::Vector3 &pos, double &yaw);

  void complite_setpoint(const std::string &frame_id,
                         const std::optional<double> &x,
                         const std::optional<double> &y,
                         const std::optional<double> &z,
                         const std::optional<double> &yaw,
                         const geometry_msgs::msg::PoseStamped &ref_pose,
                         geometry_msgs::msg::PoseStamped &pose) const;

  void check_fcu(state &process_state);

  void publish_offboard();

  void publish_position();
  void publish_velocity();
  void update_navigation_setpoint();
  void check_action_complite();

  void change_state(const state new_state);

  rclcpp::Logger get_logger() const { return m_logger; }
  rclcpp::Clock::SharedPtr get_clock() const { return m_clock; }

  clover2_offboard::backend::base_backend::SharedPtr m_backend;

  state m_state{state::none};
  std::string m_local_frame{"map"};
  std::string m_setpoint_frame{"setpoint"};

  double m_speed{0.3};
  double m_tolerance{0.05};
  double m_slowdown_distance{0.3};
  std::chrono::milliseconds m_publish_period{20};
  geometry_msgs::msg::PoseStamped m_nav_setpoint;
  geometry_msgs::msg::PoseStamped m_pose_setpoint;
  geometry_msgs::msg::Twist m_velocity_setpoint;

  rclcpp::Logger m_logger;
  rclcpp::Clock::SharedPtr m_clock;
  std::shared_ptr<tf2_ros::Buffer> m_tf_buffer;
  std::shared_ptr<tf2_ros::TransformListener> m_tf_listener;
  rclcpp::CallbackGroup::SharedPtr m_group;
  rclcpp::TimerBase::SharedPtr m_update_timer;
};

} // namespace clover2_offboard
