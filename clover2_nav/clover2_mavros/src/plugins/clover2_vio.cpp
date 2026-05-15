// MAVROS
#include <mavros/frame_tf.hpp>
#include <mavros/mavros_uas.hpp>
#include <mavros/plugin.hpp>
#include <mavros/plugin_filter.hpp>
#include <mavros/utils.hpp>

// ROS
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
#include <tf2/exceptions.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

// STL
#include <string>

namespace clover2_mavros {

namespace ftf = mavros::ftf;

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
            "frame_id", "map",
            [&](const rclcpp::Parameter& p) { m_frame_id = p.as_string(); });

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

    Subscriptions get_subscriptions() override {
        return {make_handler(&clover2_vio::handle_local_position_ned)};
    }

private:
    bool is_vio_reset(const Eigen::Vector3d& p, const rclcpp::Time& stamp) {
        double jump = (p - m_last_vio_position).norm();

        if (m_first_pose) {
            m_first_pose = false;
            m_last_vio_position = p;
            return false;
        }

        m_last_vio_position = p;

        if ((stamp - m_last_pose_recv) > m_pose_gap_reset) {
            return true;
        }

        if (jump > 2.0) {
            return true;
        }

        return false;
    }

    void send_vision_estimate(
        const rclcpp::Time& stamp, const Eigen::Affine3d& tr,
        const geometry_msgs::msg::PoseWithCovariance::_covariance_type& cov) {
        if (m_last_pose_recv == stamp) {
            RCLCPP_DEBUG_THROTTLE(
                get_logger(), *get_clock(), 10,
                "Vision: Same transform as last one, dropped.");
            return;
        }

        if ((stamp - m_last_pose_recv) > m_pose_gap_reset) {
            RCLCPP_WARN(get_logger(), "Increase reset counter.");
            ++m_reset_counter;
        }

        m_last_pose_recv = stamp;

        bool reset = is_vio_reset(tr.translation(), stamp);

        if (reset) {
            RCLCPP_WARN(get_logger(), "VIO reset detected → updating offset");

            Eigen::Affine3d world_before = m_offset * tr;
            m_offset = world_before * tr.inverse();
            m_offset_initialized = true;

            ++m_reset_counter;
        }

        Eigen::Affine3d corrected = m_offset * tr;
        auto position = ftf::transform_frame_enu_ned(
            Eigen::Vector3d(corrected.translation()));

        Eigen::Quaterniond q(m_offset.rotation() * tr.rotation());
        auto rpy = ftf::quaternion_to_rpy(ftf::transform_orientation_enu_ned(
            ftf::transform_orientation_baselink_aircraft(q)));

        auto cov_ned = ftf::transform_frame_enu_ned(cov);
        ftf::EigenMapConstCovariance6d cov_map(cov_ned.data());

        mavlink::common::msg::VISION_POSITION_ESTIMATE vp{};
        vp.usec = get_time_usec(stamp);
        vp.reset_counter = m_reset_counter;

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

    void handle_local_position_ned(
        const mavlink::mavlink_message_t* msg [[maybe_unused]],
        mavlink::common::msg::LOCAL_POSITION_NED& pos_ned,
        mavros::plugin::filter::SystemAndOk filter [[maybe_unused]]) {
        m_local_position = ftf::transform_frame_ned_enu(
            Eigen::Vector3d(pos_ned.x, pos_ned.y, pos_ned.z));

        auto enu_orientation_msg = uas->data.get_attitude_orientation_enu();
        tf2::fromMsg(enu_orientation_msg, m_local_orientation);
    }

    uint8_t m_reset_counter;
    std::string m_frame_id;
    std::string m_pose_child_frame_id;

    std::atomic<bool> m_first_pose = true;
    std::atomic<bool> m_offset_initialized = false;

    Eigen::Vector3d m_local_position;
    Eigen::Quaterniond m_local_orientation;
    Eigen::Affine3d m_offset = Eigen::Affine3d::Identity();
    Eigen::Vector3d m_last_vio_position = Eigen::Vector3d::Zero();

    rclcpp::Time m_last_pose_recv{0, 0, RCL_ROS_TIME};
    rclcpp::Duration m_pose_gap_reset{rclcpp::Duration::from_seconds(1.0)};

    rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::
        SharedPtr m_pose_cov_sub;
};
}  // namespace clover2_mavros

#include <mavros/mavros_plugin_register_macro.hpp>
MAVROS_PLUGIN_REGISTER(clover2_mavros::clover2_vio)
