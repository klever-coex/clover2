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
        , m_reset_counter(0)
        , m_pose_child_frame_id(uas_->get_base_link_frame_id())
        , m_pose_gap_reset(rclcpp::Duration::from_seconds(1.0))
        , m_have_last_pose(false) {
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

    void pose_cov_cb(
        const geometry_msgs::msg::PoseWithCovarianceStamped::SharedPtr
            pose_msg) {
        const rclcpp::Time now = node->now();
        if (m_have_last_pose && m_pose_gap_reset.seconds() > 0.0) {
            if ((now - m_last_pose_recv) > m_pose_gap_reset) {
                ++m_reset_counter;
            }
        }
        m_last_pose_recv = now;
        m_have_last_pose = true;

        Eigen::Affine3d tf_parent2parent_des = Eigen::Affine3d::Identity();
        Eigen::Affine3d tf_child2child_des = Eigen::Affine3d::Identity();

        const std::string& parent_frame = pose_msg->header.frame_id;
        const bool parent_tf_available = lookup_static_transform(
            parent_frame + "_ned", parent_frame, tf_parent2parent_des);
        const bool child_tf_available =
            lookup_static_transform(m_pose_child_frame_id + "_frd",
                                    m_pose_child_frame_id, tf_child2child_des);
        if (!parent_tf_available || !child_tf_available) {
            return;
        }

        ftf::Covariance6d cov_pose_arr = pose_msg->pose.covariance;
        ftf::EigenMapCovariance6d cov_pose_map(cov_pose_arr.data());

        Matrix6d r_pose = Matrix6d::Zero();
        Matrix6d r_vel = Matrix6d::Zero();

        const Eigen::Vector3d position =
            tf_parent2parent_des.linear() *
            ftf::to_eigen(pose_msg->pose.pose.position);

        const Eigen::Quaterniond q_child2parent(
            ftf::to_eigen(pose_msg->pose.pose.orientation));
        const Eigen::Affine3d tf_childDes2parentDes =
            tf_parent2parent_des * q_child2parent *
            tf_child2child_des.inverse();
        const Eigen::Quaterniond orientation(tf_childDes2parentDes.linear());

        r_pose.block<3, 3>(0, 0) = r_pose.block<3, 3>(3, 3) =
            tf_parent2parent_des.linear();
        r_vel.block<3, 3>(0, 0) = r_vel.block<3, 3>(3, 3) =
            tf_child2child_des.linear();
        cov_pose_map = r_pose * cov_pose_map * r_pose.transpose();

        Matrix6d cov_vel_unknown = Matrix6d::Zero();
        cov_vel_unknown(0, 0) = cov_vel_unknown(1, 1) = cov_vel_unknown(2, 2) =
            1e6;
        cov_vel_unknown(3, 3) = cov_vel_unknown(4, 4) = cov_vel_unknown(5, 5) =
            1e4;
        Eigen::Matrix<double, 6, 6, Eigen::RowMajor> cov_vel_map =
            r_vel * cov_vel_unknown * r_vel.transpose();

        mavlink::common::msg::ODOMETRY msg{};
        msg.frame_id = utils::enum_value(MAV_FRAME::LOCAL_FRD);
        msg.child_frame_id = utils::enum_value(MAV_FRAME::BODY_FRD);
        msg.estimator_type = utils::enum_value(MAV_ESTIMATOR_TYPE::VISION);
        msg.time_usec = get_time_usec(pose_msg->header.stamp);
        msg.reset_counter = m_reset_counter;

        msg.x = static_cast<float>(position.x());
        msg.y = static_cast<float>(position.y());
        msg.z = static_cast<float>(position.z());
        msg.vx = msg.vy = msg.vz = 0.F;
        msg.rollspeed = msg.pitchspeed = msg.yawspeed = 0.F;

        ftf::quaternion_to_mavlink(orientation, msg.q);
        ftf::covariance_urt_to_mavlink(cov_pose_map, msg.pose_covariance);
        ftf::covariance_urt_to_mavlink(cov_vel_map, msg.velocity_covariance);

        uas->send_message(msg);
    }

    uint8_t m_reset_counter;
    std::string m_pose_child_frame_id;
    rclcpp::Duration m_pose_gap_reset{rclcpp::Duration::from_seconds(1.0)};
    bool m_have_last_pose;
    rclcpp::Time m_last_pose_recv{0, 0, RCL_ROS_TIME};

    rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::
        SharedPtr m_pose_cov_sub;
};
}  // namespace clover2_mavros

#include <mavros/mavros_plugin_register_macro.hpp>
MAVROS_PLUGIN_REGISTER(clover2_mavros::clover2_vio)
