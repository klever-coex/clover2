#pragma once

// clover2
#include <clover2_fcu/data/system_state.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

// STL
#include <memory>
#include <utility>

namespace clover2_fcu::capability {

class system_state_interface : public telemetry_interface<data::system_state> {
public:
    using shared_ptr = std::shared_ptr<system_state_interface>;

    static constexpr const char* name = "system_state";

    explicit system_state_interface(rclcpp::Clock::SharedPtr clock)
        : telemetry_interface<data::system_state>(std::move(clock)) {}
};

}  // namespace clover2_fcu::capability
