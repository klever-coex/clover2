#pragma once

#include <clover2_fcu_bridge/backend/base_backend.hpp>
#include <clover2_fcu_bridge/data/mode.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

#include <memory>

namespace clover2_fcu_bridge {

class fcu_backend {
public:
    explicit fcu_backend(std::weak_ptr<backend::base_backend> backend);

    bool ready() const;
    bool connected() const;

    bool is_armed() const;
    data::mode get_mode() const;

    void arm();
    void disarm();
    void set_mode(const data::mode& mode);

    geometry_msgs::msg::PoseStamped get_pose() const;
    void set_position_setpoint(double x, double y, double z, double yaw);
    void set_velocity_setpoint(double vx, double vy, double vz,
                               double yaw_rate);

private:
    std::shared_ptr<backend::base_backend> lock() const;

    std::weak_ptr<backend::base_backend> m_backend;
};

}  // namespace clover2_fcu_bridge
