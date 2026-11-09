// clover2
#include <clover2_fcu_mavros/capability/battery.hpp>

namespace clover2_fcu_mavros::capability {

namespace {

constexpr uint8_t k_primary_id = 0;

/// Exposes the protected update_data of the telemetry base to the battery
/// provider.
class battery_source_impl : public battery_source {
public:
    using battery_source::battery_source;

    void inject(const clover2_fcu::data::battery& sample) {
        update_data(sample);
    }
};

clover2_fcu::data::battery convert(const sensor_msgs::msg::BatteryState& msg) {
    clover2_fcu::data::battery battery;
    battery.voltage_v = msg.voltage;
    battery.current_a = msg.current;
    battery.charge_percent = msg.percentage * 100.0;
    return battery;
}

}  // namespace

battery::battery(const clover2_fcu::context& ctx, std::string prefix)
    : clover2_fcu::capability::battery_interface(ctx.clock)
    , m_clock(ctx.clock) {
    m_sub = ctx.subscribe<sensor_msgs::msg::BatteryState>(
        prefix + "/battery", rclcpp::SensorDataQoS(),
        [this](sensor_msgs::msg::BatteryState::SharedPtr msg) {
            handle_battery(*msg);
        });
}

std::vector<uint8_t> battery::ids() const {
    // Only the primary exists on this transport; visible as data arrives.
    return has_data() ? std::vector<uint8_t>{k_primary_id}
                      : std::vector<uint8_t>{};
}

battery_source::shared_ptr battery::source(uint8_t id) {
    const std::scoped_lock lock(m_mutex);
    auto& slot = m_sources[id];
    if (!slot) {
        slot = std::make_shared<battery_source_impl>(m_clock, id);
    }
    return slot;
}

void battery::handle_battery(const sensor_msgs::msg::BatteryState& msg) {
    battery_source::shared_ptr primary_source;
    {
        const std::scoped_lock lock(m_mutex);
        auto& slot = m_sources[k_primary_id];
        if (!slot) {
            slot = std::make_shared<battery_source_impl>(m_clock, k_primary_id);
        }
        primary_source = slot;
    }

    const auto sample = convert(msg);

    // update_data fires subscriber callbacks — never under the provider
    // mutex (threading rule 3).
    if (primary_source) {
        std::static_pointer_cast<battery_source_impl>(primary_source)
            ->inject(sample);
    }
    update_data(sample);
}

}  // namespace clover2_fcu_mavros::capability
