#pragma once

// clover2
#include <clover2_fcu/common/command_handle.hpp>
#include <clover2_fcu/data/flight_mode.hpp>

// STL
#include <memory>

namespace clover2_fcu::capability {

/**
 * @brief Command capability contract.
 *
 * Every command returns an async completion handle; there are no
 * callback parameters and no synchronous results:
 *
 *   auto h = fcu.commands()->land();
 *   h->on_done([](result r) { ... });  // navigation layer sequencing
 *   h->wait_for(30s);                   // scripts/tests only
 *   h->cancel();                        // explicit abort (tasks)
 *
 * Two flavors share one mechanics:
 * - request commands (arm/disarm/set_mode) resolve on the transport
 *   ack; cancel() merely detaches;
 * - task commands (takeoff/land/rtl) resolve on the observed flight
 *   state and are aborted by cancel() (abort semantics are defined by
 *   the backend, e.g. a mode switch);
 *
 * The handle is kept alive by its transport keeper until a terminal
 * state: dropping the returned pointer never cancels the command.
 */
class commands_interface {
public:
    using shared_ptr = std::shared_ptr<commands_interface>;

    static constexpr const char* name = "commands";

    virtual ~commands_interface() = default;

    /// @brief Arm the vehicle (request).
    virtual command_handle::shared_ptr arm() = 0;

    /// @brief Disarm the vehicle (request).
    virtual command_handle::shared_ptr disarm() = 0;

    /// @brief Switch the flight mode (request).
    virtual command_handle::shared_ptr set_mode(data::flight_mode mode) = 0;

    /// @brief Trigger an automatic takeoff (task).
    virtual command_handle::shared_ptr takeoff() = 0;

    /// @brief Trigger an automatic landing (task).
    virtual command_handle::shared_ptr land() = 0;

    /// @brief Trigger a return-to-launch (task).
    virtual command_handle::shared_ptr rtl() = 0;
};

}  // namespace clover2_fcu::capability
