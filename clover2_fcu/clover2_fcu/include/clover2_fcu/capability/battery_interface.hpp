#pragma once

// clover2
#include <clover2_fcu/data/battery.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

// STL
#include <cstdint>
#include <memory>
#include <vector>

namespace clover2_fcu::capability {

/// @brief A single battery source: standard telemetry plus the observed
///        battery id (logs / observability). When the battery disappears,
///        the source stays alive and last_valid() goes false.
class battery_source : public telemetry_interface<data::battery> {
public:
    using shared_ptr = std::shared_ptr<battery_source>;

    battery_source(rclcpp::Clock::SharedPtr clock, uint8_t id)
        : telemetry_interface<data::battery>(std::move(clock))
        , m_id(id) {}

    uint8_t id() const { return m_id; }

private:
    uint8_t m_id;
};

/// @brief Multi-instance telemetry: a container of lazy per-id sources,
///        itself remaining the telemetry of the primary battery — the one
///        the FCU marks primary. Existing code (fcu.battery()->last())
///        never notices the multi-battery semantics.
class battery_interface : public telemetry_interface<data::battery> {
public:
    using shared_ptr = std::shared_ptr<battery_interface>;

    static constexpr const char* name = "battery";

    explicit battery_interface(rclcpp::Clock::SharedPtr clock)
        : telemetry_interface<data::battery>(std::move(clock)) {}

    /// @brief Live ids (as data arrives; empty before the first message).
    ///        Appearance/disappearance is observed by polling.
    virtual std::vector<uint8_t> ids() const = 0;

    /// @brief A view of one battery: created lazily, filled when data
    ///        arrives (has_data() == false until then). Always valid —
    ///        forgiving API.
    virtual battery_source::shared_ptr source(uint8_t id) = 0;
};

}  // namespace clover2_fcu::capability
