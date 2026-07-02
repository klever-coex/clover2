#pragma once

#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2/LinearMath/Vector3.hpp>

namespace clover2_fcu_bridge {

class navigation_controller {
public:
    void set_tolerance(double tolerance);
    void set_slowdown_distance(double distance);
    void set_speed(double speed);
    void set_speed_limit(double speed);
    void set_yaw_rate(double yaw_rate);

    double get_tolerance() const { return m_tolerance; }
    double get_slowdown_distance() const { return m_slowdown; }
    double get_speed() const { return m_speed; }
    double get_speed_limit() const { return m_speed_limit; }
    double get_yaw_rate() const { return m_yaw_rate; }

    void reset();
    void set_target(const geometry_msgs::msg::PoseStamped& target,
                    double speed);

    void update(const geometry_msgs::msg::PoseStamped& current_pose);

    const geometry_msgs::msg::PoseStamped& current_setpoint() const {
        return m_setpoint;
    }

    bool target_reached() const { return m_target_reached; }

    static void pose_diff(const geometry_msgs::msg::PoseStamped& current,
                          const geometry_msgs::msg::PoseStamped& target,
                          tf2::Vector3& diff_pos, double& diff_yaw);

private:
    static void extract_pose(const geometry_msgs::msg::PoseStamped& pose,
                             tf2::Vector3& pos, double& yaw);
    static void set_yaw(geometry_msgs::msg::Quaternion& q, double yaw);
    static bool pose_finite(const geometry_msgs::msg::PoseStamped& pose);
    static double normalize_angle(double a);

    geometry_msgs::msg::PoseStamped m_target;
    geometry_msgs::msg::PoseStamped m_setpoint;

    double m_speed{0.3};
    double m_speed_limit{1.0};
    double m_yaw_rate{0.1};
    double m_tolerance{0.25};
    double m_slowdown{0.5};

    bool m_has_target{false};
    bool m_target_reached{false};
};

}  // namespace clover2_fcu_bridge
