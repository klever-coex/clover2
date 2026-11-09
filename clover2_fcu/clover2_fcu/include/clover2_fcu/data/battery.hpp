#pragma once

namespace clover2_fcu::data {

/// One battery sample (multi-battery views are per-id, see battery_source).
struct battery {
    double voltage_v{0.0};
    double current_a{0.0};
    double charge_percent{0.0};
};

}  // namespace clover2_fcu::data
