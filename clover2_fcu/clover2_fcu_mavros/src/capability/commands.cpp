// clover2
#include <clover2_fcu_mavros/capability/commands.hpp>
#include <clover2_fcu_mavros/mode_mapping.hpp>

// STL
#include <limits>

namespace clover2_fcu_mavros::capability {

commands::commands(const clover2_fcu::context& ctx, std::string prefix)
    : m_logger(ctx.logger) {
    m_arming_client =
        ctx.client<mavros_msgs::srv::CommandBool>(prefix + "/cmd/arming");
    m_set_mode_client =
        ctx.client<mavros_msgs::srv::SetMode>(prefix + "/set_mode");
    m_takeoff_client =
        ctx.client<mavros_msgs::srv::CommandTOL>(prefix + "/cmd/takeoff");
    m_land_client =
        ctx.client<mavros_msgs::srv::CommandTOL>(prefix + "/cmd/land");
}

void commands::arm(result_callback cb, bool run_preflight_checks) {
    // mavros cmd/arming carries no preflight-checks flag; PX4 runs the
    // preflight checks on arm anyway.
    (void)run_preflight_checks;

    auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    request->value = true;
    invoke<mavros_msgs::srv::CommandBool>(
        "arm", m_arming_client, std::move(request), std::move(cb),
        [](const auto& r) { return r.success; });
}

void commands::disarm(result_callback cb) {
    auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    request->value = false;
    invoke<mavros_msgs::srv::CommandBool>(
        "disarm", m_arming_client, std::move(request), std::move(cb),
        [](const auto& r) { return r.success; });
}

void commands::set_mode(result_callback cb,
                        clover2_fcu::data::flight_mode mode) {
    const auto native = fcu_mode_to_mavros(mode);
    if (!native) {
        RCLCPP_WARN(m_logger,
                    "command 'set_mode': mode '%s' has no native "
                    "mavros equivalent",
                    mode.to_string().data());
        cb(false);
        return;
    }

    auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
    request->base_mode = 0;
    request->custom_mode = *native;
    invoke<mavros_msgs::srv::SetMode>(
        "set_mode", m_set_mode_client, std::move(request), std::move(cb),
        [](const auto& r) { return r.mode_sent; });
}

void commands::takeoff(result_callback cb, std::optional<double> altitude,
                       std::optional<double> heading) {
    if (!altitude) {
        RCLCPP_WARN(m_logger,
                    "command 'takeoff' rejected: altitude is required");
        cb(false);
        return;
    }

    // PX4 treats a NaN yaw in NAV_TAKEOFF/LAND as "keep the current
    // heading"; a plain 0 would turn the vehicle north.
    auto request = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
    request->min_pitch = 0.f;
    request->yaw = heading ? static_cast<float>(*heading)
                           : std::numeric_limits<float>::quiet_NaN();
    request->latitude = 0.f;
    request->longitude = 0.f;
    request->altitude = static_cast<float>(*altitude);
    invoke<mavros_msgs::srv::CommandTOL>(
        "takeoff", m_takeoff_client, std::move(request), std::move(cb),
        [](const auto& r) { return r.success; });
}

void commands::land(result_callback cb) {
    auto request = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
    request->min_pitch = 0.f;
    request->yaw = std::numeric_limits<float>::quiet_NaN();
    request->latitude = 0.f;
    request->longitude = 0.f;
    request->altitude = 0.f;
    invoke<mavros_msgs::srv::CommandTOL>(
        "land", m_land_client, std::move(request), std::move(cb),
        [](const auto& r) { return r.success; });
}

void commands::rtl(result_callback cb) {
    // RTL is not a mavros command service — switch into the native RTL mode.
    set_mode(std::move(cb), clover2_fcu::data::flight_mode::rtl());
}

}  // namespace clover2_fcu_mavros::capability
