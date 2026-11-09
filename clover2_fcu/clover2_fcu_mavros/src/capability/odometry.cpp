// clover2
#include <clover2_fcu_mavros/capability/odometry.hpp>

namespace clover2_fcu_mavros::capability {

namespace {

clover2_fcu::data::odometry convert(const nav_msgs::msg::Odometry& msg) {
    clover2_fcu::data::odometry odom;
    odom.position =
        Eigen::Vector3d(msg.pose.pose.position.x, msg.pose.pose.position.y,
                        msg.pose.pose.position.z);
    odom.velocity =
        Eigen::Vector3d(msg.twist.twist.linear.x, msg.twist.twist.linear.y,
                        msg.twist.twist.linear.z);
    odom.attitude = Eigen::Quaterniond(
        msg.pose.pose.orientation.w, msg.pose.pose.orientation.x,
        msg.pose.pose.orientation.y, msg.pose.pose.orientation.z);
    odom.angular_velocity =
        Eigen::Vector3d(msg.twist.twist.angular.x, msg.twist.twist.angular.y,
                        msg.twist.twist.angular.z);
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
