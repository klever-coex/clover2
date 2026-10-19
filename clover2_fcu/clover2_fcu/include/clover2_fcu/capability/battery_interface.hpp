/**
 * @file battery_interface.hpp
 * @brief Provides the battery capability contract.
 */

#pragma once

// clover2
#include <clover2_fcu/data/battery.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

namespace clover2_fcu::capability {

/**
 * @brief Battery telemetry capability contract.
 *
 * Served by a backend provider.
 */
class battery_interface : public telemetry_interface<data::battery> {
public:
    using shared_ptr = std::shared_ptr<battery_interface>;

    static constexpr const char* name = "battery";

    explicit battery_interface(base_interface& base)
        : telemetry_interface<data::battery>(base) {}
};

}  // namespace clover2_fcu::capability
