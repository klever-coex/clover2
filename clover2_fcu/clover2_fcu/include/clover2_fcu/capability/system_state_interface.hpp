#pragma once

// clover2
#include <clover2_fcu/data/system_state.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

namespace clover2_fcu::capability {

class system_state_interface : public telemetry_interface<data::system_state> {
public:
    using shared_ptr = std::shared_ptr<system_state_interface>;

    static constexpr const char* name = "system_state";

    explicit system_state_interface(base_interface::shared_ptr base)
        : telemetry_interface<data::system_state>(base) {}
};

}  // namespace clover2_fcu::capability
