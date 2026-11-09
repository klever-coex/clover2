#pragma once

// clover2
#include <clover2_fcu/capability/estimator_interface.hpp>
#include <clover2_fcu/context.hpp>

// ROS2
#include <rclcpp/publisher.hpp>

// msgs
#include <geographic_msgs/msg/geo_point_stamped.hpp>
#include <mavros_msgs/msg/optical_flow.hpp>
#include <nav_msgs/msg/odometry.hpp>

// STL
#include <mutex>
#include <string>

namespace clover2_fcu_mavros::capability {

using clover2_fcu::result;

class estimator : public clover2_fcu::capability::estimator_interface {
public:
    explicit estimator(const clover2_fcu::context& ctx, std::string prefix);

    result reset(clover2_fcu::capability::estimator_reset scope) override;

    result set_origin(const clover2_fcu::data::geo_point& origin) override;

    clover2_fcu::utils::sensor_publisher<clover2_fcu::data::odometry>
    odometry_publisher(
        clover2_fcu::capability::estimator_source source) override;

    clover2_fcu::utils::sensor_publisher<clover2_fcu::data::optical_flow>
    optical_flow_publisher() override;

private:
    template <typename T, typename Publish>
    clover2_fcu::utils::sensor_publisher<T> make_sensor(Publish publish) {
        std::weak_ptr<clover2_fcu::telemetry_interface<
            clover2_fcu::data::estimator_status>>
            self = weak_from_this();
        return clover2_fcu::utils::sensor_publisher<T>(
            0, [self = std::move(self),
                publish = std::move(publish)](const T& sample) {
                if (const auto provider = self.lock()) {
                    publish(*std::static_pointer_cast<estimator>(provider),
                            sample);
                }
            });
    }

    void publish_odometry(const clover2_fcu::data::odometry& sample);
    void publish_optical_flow(const clover2_fcu::data::optical_flow& sample);

    clover2_fcu::context m_context;
    rclcpp::Clock::SharedPtr m_clock;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr m_odom_pub;
    rclcpp::Publisher<geographic_msgs::msg::GeoPointStamped>::SharedPtr
        m_origin_pub;
    std::string m_flow_topic;

    std::mutex m_mutex;
    rclcpp::Publisher<mavros_msgs::msg::OpticalFlow>::SharedPtr m_flow_pub;
};

}  // namespace clover2_fcu_mavros::capability
