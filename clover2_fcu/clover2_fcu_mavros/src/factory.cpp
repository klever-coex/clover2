// clover2
#include <clover2_fcu_mavros/capability/battery.hpp>
#include <clover2_fcu_mavros/capability/commands.hpp>
#include <clover2_fcu_mavros/capability/estimator.hpp>
#include <clover2_fcu_mavros/capability/odometry.hpp>
#include <clover2_fcu_mavros/capability/parameters.hpp>
#include <clover2_fcu_mavros/capability/setpoints.hpp>
#include <clover2_fcu_mavros/capability/system_state.hpp>
#include <clover2_fcu_mavros/factory.hpp>
#include <clover2_fcu_mavros/topic_prefix.hpp>

// third party
#include <pluginlib/class_list_macros.hpp>

namespace clover2_fcu_mavros {

namespace {

namespace provider_capability = clover2_fcu::capability;

}  // namespace

factory::factory() {
    add<provider_capability::system_state_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::system_state>(
                ctx, topic_prefix(ctx));
        });
    add<provider_capability::odometry_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::odometry>(ctx,
                                                          topic_prefix(ctx));
        });
    add<provider_capability::battery_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::battery>(ctx,
                                                         topic_prefix(ctx));
        });
    add<provider_capability::commands_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::commands>(ctx,
                                                          topic_prefix(ctx));
        });
    add<provider_capability::parameters_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::parameters>(ctx,
                                                            topic_prefix(ctx));
        });
    add<provider_capability::estimator_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::estimator>(ctx,
                                                           topic_prefix(ctx));
        });
    add<provider_capability::setpoint_position_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::position_setpoint>(
                ctx, topic_prefix(ctx));
        });
    add<provider_capability::setpoint_velocity_interface>(
        [](const clover2_fcu::context& ctx) {
            return std::make_shared<capability::velocity_setpoint>(
                ctx, topic_prefix(ctx));
        });
}

std::string factory::protocol() const { return "mavros"; }

void factory::do_initialize() {}

}  // namespace clover2_fcu_mavros

PLUGINLIB_EXPORT_CLASS(clover2_fcu_mavros::factory,
                       clover2_fcu::backend::backend_base)
