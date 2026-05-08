#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
#include <mavros/frame_tf.hpp>
#include <mavros/mavros_uas.hpp>
#include <mavros/plugin.hpp>
#include <mavros/utils.hpp>
#include <tf2/exceptions.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

#include <string>

namespace clover2_mavros {

namespace ftf = mavros::ftf;
namespace utils = mavros::utils;

using mavlink::common::MAV_ESTIMATOR_TYPE;
using mavlink::common::MAV_FRAME;
using Matrix6d = Eigen::Matrix<double, 6, 6, Eigen::RowMajor>;

class clover2_vio : public mavros::plugin::Plugin {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    explicit clover2_vio(mavros::plugin::UASPtr uas_)
        : Plugin(uas_, "clover2_vio")
        , m_reset_counter(10)
        , m_pose_child_frame_id(uas_->get_base_link_frame_id())
        , m_pose_gap_reset(rclcpp::Duration::from_seconds(1.0)) {
        enable_node_watch_parameters();

        node_declare_and_watch_parameter(
            "pose.child_frame_id", uas_->get_base_link_frame_id(),
            [&](const rclcpp::Parameter& p) {
                m_pose_child_frame_id = p.as_string();
            });
        node_declare_and_watch_parameter(
            "pose.gap_reset_s", 1.0, [&](const rclcpp::Parameter& p) {
                m_pose_gap_reset =
                    rclcpp::Duration::from_seconds(p.as_double());
            });

        m_pose_cov_sub = node->create_subscription<
            geometry_msgs::msg::PoseWithCovarianceStamped>(
            "~/pose_cov", 1,
            std::bind(&clover2_vio::pose_cov_cb, this, std::placeholders::_1));
    }

    Subscriptions get_subscriptions() override { return {}; }

private:
    bool lookup_static_transform(const std::string& target,
                                 const std::string& source,
                                 Eigen::Affine3d& tf_source2target) {
        tf_source2target = Eigen::Affine3d::Identity();

        try {
            tf_source2target =
                tf2::transformToEigen(uas->tf2_buffer.lookupTransform(
                    target, source, rclcpp::Time(0)));
            return true;
        } catch (tf2::TransformException& ex) {
            RCLCPP_ERROR_THROTTLE(
                get_logger(), *get_clock(), 1,
                "clover2_vio: static transform %s -> %s unavailable: %s",
                source.c_str(), target.c_str(), ex.what());
            return false;
        }
    }

    void send_vision_estimate(
        const rclcpp::Time& stamp, const Eigen::Affine3d& tr,
        const geometry_msgs::msg::PoseWithCovariance::_covariance_type& cov) {
        auto position =
            ftf::transform_frame_enu_ned(Eigen::Vector3d(tr.translation()));

        Eigen::Quaterniond q(tr.rotation());
        if (!std::isfinite(q.x()) ||
            !std::isfinite(q.y()) ||
            !std::isfinite(q.z()) ||
            !std::isfinite(q.w()))
        {
            RCLCPP_ERROR(get_logger(),
                "Vision: quaternion NaN/Inf");
            return;
        }

        double q_norm = q.norm();

        if (std::abs(q_norm - 1.0) > 0.05) {
            RCLCPP_ERROR(get_logger(),
                "Vision: invalid quaternion norm: %.3f",
                q_norm);
            return;
        }

        auto rpy = ftf::quaternion_to_rpy(ftf::transform_orientation_enu_ned(
            ftf::transform_orientation_baselink_aircraft(q)));

        auto cov_ned = ftf::transform_frame_enu_ned(cov);
        ftf::EigenMapConstCovariance6d cov_map(cov_ned.data());

        mavlink::common::msg::VISION_POSITION_ESTIMATE vp{};
        vp.usec = get_time_usec(stamp);
        // vp.reset_counter = m_reset_counter;

        vp.x = position.x();
        vp.y = position.y();
        vp.z = position.z();
        vp.roll = rpy.x();
        vp.pitch = rpy.y();
        vp.yaw = rpy.z();

        ftf::covariance_urt_to_mavlink(cov_map, vp.covariance);

        uas->send_message(vp);
    }

    void pose_cov_cb(
        const geometry_msgs::msg::PoseWithCovarianceStamped::SharedPtr
            pose_msg) {
        const rclcpp::Time stamp = rclcpp::Time(pose_msg->header.stamp);

        if (m_last_pose_recv == stamp) {
            RCLCPP_DEBUG_THROTTLE(
                get_logger(), *get_clock(), 10,
                "Vision: Same transform as last one, dropped.");
            return;
        }

        if ((stamp - m_last_pose_recv) > m_pose_gap_reset) {
            RCLCPP_WARN(get_logger(),
                        "Increase reset counter.");
            ++m_reset_counter;
        }

        m_last_pose_recv = stamp;

        Eigen::Affine3d tr;
        tf2::fromMsg(pose_msg->pose.pose, tr);

        for (const auto& v : pose_msg->pose.covariance) {
            if (!std::isfinite(v)) {
                RCLCPP_ERROR(get_logger(),
                    "Vision: covariance contains NaN/Inf");
                return;
            }
        }

        send_vision_estimate(stamp, tr, pose_msg->pose.covariance);
    }

    uint8_t m_reset_counter;
    std::string m_pose_child_frame_id;
    rclcpp::Time m_last_pose_recv{0, 0, RCL_ROS_TIME};
    rclcpp::Duration m_pose_gap_reset{rclcpp::Duration::from_seconds(1.0)};

    rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::
        SharedPtr m_pose_cov_sub;
};
}  // namespace clover2_mavros

#include <mavros/mavros_plugin_register_macro.hpp>
MAVROS_PLUGIN_REGISTER(clover2_mavros::clover2_vio)
