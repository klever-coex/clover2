// clover2
#include <clover2_fcu_mavros/capability/estimator.hpp>

// STL
#include <cmath>
#include <utility>

namespace clover2_fcu_mavros::capability {

namespace {

using clover2_fcu::data::odometry;
using clover2_fcu::data::optical_flow;

nav_msgs::msg::Odometry convert(const odometry& sample,
                                const rclcpp::Time& now) {
    nav_msgs::msg::Odometry msg;
    msg.header.frame_id = "odom";
    msg.header.stamp = sample.stamp.nanoseconds() == 0 ? now : sample.stamp;
    msg.child_frame_id = "base_link";

    msg.pose.pose.position.x = sample.position.x();
    msg.pose.pose.position.y = sample.position.y();
    msg.pose.pose.position.z = sample.position.z();
    msg.pose.pose.orientation.w = sample.attitude.w();
    msg.pose.pose.orientation.x = sample.attitude.x();
    msg.pose.pose.orientation.y = sample.attitude.y();
    msg.pose.pose.orientation.z = sample.attitude.z();

    msg.twist.twist.linear.x = sample.velocity.x();
    msg.twist.twist.linear.y = sample.velocity.y();
    msg.twist.twist.linear.z = sample.velocity.z();
    msg.twist.twist.angular.x = sample.angular_velocity.x();
    msg.twist.twist.angular.y = sample.angular_velocity.y();
    msg.twist.twist.angular.z = sample.angular_velocity.z();

    return msg;
}

mavros_msgs::msg::OpticalFlow convert(const optical_flow& sample,
                                      const rclcpp::Time& now) {
    mavros_msgs::msg::OpticalFlow msg;
    msg.header.stamp = sample.stamp.nanoseconds() == 0 ? now : sample.stamp;
    msg.flow.x = sample.integrated_x.value_or(0.0);
    msg.flow.y = sample.integrated_y.value_or(0.0);
    msg.flow_rate.x = sample.integrated_xgyro.value_or(0.0);
    msg.flow_rate.y = sample.integrated_ygyro.value_or(0.0);
    msg.flow_rate.z = sample.integrated_zgyro.value_or(0.0);
    msg.quality = sample.quality;
    msg.ground_distance = static_cast<float>(sample.distance.value_or(0.0));

    return msg;
}

}  // namespace

estimator::estimator(const clover2_fcu::context& ctx, std::string prefix)
    : clover2_fcu::capability::estimator_interface(ctx.clock)
    , m_context(ctx)
    , m_clock(ctx.clock)
    , m_odom_pub(m_context.publisher<nav_msgs::msg::Odometry>(
          prefix + "/odometry/out", rclcpp::QoS(10)))
    , m_origin_pub(m_context.publisher<geographic_msgs::msg::GeoPointStamped>(
          prefix + "/global_position/set_gp_origin", rclcpp::QoS(10)))
    , m_flow_topic(prefix + "/optical_flow/raw/send") {}

result estimator::reset(clover2_fcu::capability::estimator_reset scope) {
    (void)scope;
    return result::value::unsupported;
}

result estimator::set_origin(const clover2_fcu::data::geo_point& origin) {
    geographic_msgs::msg::GeoPointStamped msg;
    msg.header.stamp = m_clock->now();
    msg.position.latitude = origin.lat * 180.0 / M_PI;
    msg.position.longitude = origin.lon * 180.0 / M_PI;
    msg.position.altitude = origin.alt;

    m_origin_pub->publish(std::move(msg));
    return result::value::success;
}

clover2_fcu::utils::sensor_publisher<clover2_fcu::data::odometry>
estimator::odometry_publisher(
    clover2_fcu::capability::estimator_source source) {
    (void)source;
    return make_sensor<clover2_fcu::data::odometry>(
        [](estimator& self, const clover2_fcu::data::odometry& sample) {
            self.publish_odometry(sample);
        });
}

clover2_fcu::utils::sensor_publisher<clover2_fcu::data::optical_flow>
estimator::optical_flow_publisher() {
    const std::scoped_lock lock(m_mutex);
    if (m_flow_pub) {
        return clover2_fcu::utils::sensor_publisher<
            clover2_fcu::data::optical_flow>{};
    }

    m_flow_pub = m_context.publisher<mavros_msgs::msg::OpticalFlow>(
        m_flow_topic, rclcpp::QoS(1));

    return make_sensor<clover2_fcu::data::optical_flow>(
        [](estimator& self, const clover2_fcu::data::optical_flow& sample) {
            self.publish_optical_flow(sample);
        });
}

void estimator::publish_odometry(const clover2_fcu::data::odometry& sample) {
    m_odom_pub->publish(convert(sample, m_clock->now()));
}

void estimator::publish_optical_flow(
    const clover2_fcu::data::optical_flow& sample) {
    m_flow_pub->publish(convert(sample, m_clock->now()));
}

}  // namespace clover2_fcu_mavros::capability
