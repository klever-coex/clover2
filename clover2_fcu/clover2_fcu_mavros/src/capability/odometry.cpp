// clover2
#include <clover2_fcu_mavros/capability/odometry.hpp>

// ROS2
#include <tf2_eigen/tf2_eigen.hpp>

namespace clover2_fcu_mavros::capability {

namespace {

clover2_fcu::data::odometry convert(const nav_msgs::msg::Odometry& msg) {
    clover2_fcu::data::odometry odom;

    tf2::fromMsg(msg.pose.pose.position, odom.position);
    tf2::fromMsg(msg.twist.twist.linear, odom.velocity);
    tf2::fromMsg(msg.pose.pose.orientation, odom.attitude);
    tf2::fromMsg(msg.twist.twist.angular, odom.angular_velocity);

    odom.stamp = msg.header.stamp;
    return odom;
}

}  // namespace

odometry::odometry(const clover2_fcu::context& ctx, std::string prefix)
    : clover2_fcu::capability::odometry_interface(ctx.clock) {
    m_odom_sub = ctx.subscribe<nav_msgs::msg::Odometry>(
        prefix + "/local_position/odom", rclcpp::SensorDataQoS(),
        [this](nav_msgs::msg::Odometry::SharedPtr msg) {
            update_data(convert(*msg));
        });
}

}  // namespace clover2_fcu_mavros::capability
