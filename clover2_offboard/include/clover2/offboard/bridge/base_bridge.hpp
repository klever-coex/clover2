#pragma once

// clover2
#include <clover2/offboard/bridge/creation_context.hpp>

// RCLCPP
#include <rclcpp/rclcpp.hpp>

// Eigen
#include <Eigen/Dense>

namespace clover2::offboard::bridge {

namespace data {
enum controller_type { position, velocity, body_rate };
}

class base_bridge {
public:
    template <typename NodeT>
    explicit base_bridge(const creation_context& ctx) {
        m_node = ctx.node;
    }

    virtual ~base_bridge() = default;

    Eigen::Quaterniond get_orientation() const {
        // update from child
        return m_current_orientation;
    }

    Eigen::Vector3d get_local_position() const {
        // update from child
        return m_current_position;
    }

    Eigen::Vector3d get_orientation_rpy() const {
        // update from child
        return m_current_orientation_rpy;
    }


    // virtual functions
    virtual std::string get_mode() const = 0;

    virtual void set_local_position_setpoint(const Eigen::Vector3d& position,
                                             double yaw) = 0;
    virtual void set_local_velocity_setpoint(const Eigen::Vector3d& velocity,
                                             double yaw_rate) = 0;
    virtual void set_mode(const std::string& mode) = 0;
    virtual void set_offboard_controller(data::controller_type type) = 0;
    virtual void set_speed(double speed) = 0;

    virtual void arm() = 0;
    virtual void disarm() = 0;

protected:
    Eigen::Vector3d m_current_position;
    Eigen::Vector3d m_current_orientation_rpy;
    Eigen::Quaterniond m_current_orientation;

    rclcpp::Node::SharedPtr m_node;
};

}  // namespace clover2::offboard::bridge
