// clover2
#include <clover2_fcu/capability/battery_interface.hpp>
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/capability/estimator_interface.hpp>
#include <clover2_fcu/capability/odometry_interface.hpp>
#include <clover2_fcu/capability/parameters_interface.hpp>
#include <clover2_fcu/capability/setpoint_attitude_interface.hpp>
#include <clover2_fcu/capability/setpoint_position_interface.hpp>
#include <clover2_fcu/capability/setpoint_rates_interface.hpp>
#include <clover2_fcu/capability/setpoint_velocity_interface.hpp>
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu_fake/factory.hpp>

// third party
#include <pluginlib/class_list_macros.hpp>

namespace clover2_fcu_mavros {

factory::factory() {}

std::string factory::protocol() const { return "mavros"; }

void factory::do_initialize(std::shared_ptr<capability::base_interface>) {}

}  // namespace clover2_fcu_mavros

PLUGINLIB_EXPORT_CLASS(clover2_fcu_mavros::factory,
                       clover2_fcu::backend::backend_base)
