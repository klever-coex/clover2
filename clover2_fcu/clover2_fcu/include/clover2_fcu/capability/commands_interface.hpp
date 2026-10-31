#pragma once

// clover2
#include <clover2_fcu/common/command_handle.hpp>
#include <clover2_fcu/data/flight_mode.hpp>

// STL
#include <functional>
#include <memory>

namespace clover2_fcu::capability {

class commands_interface {
public:
    using shared_ptr = std::shared_ptr<commands_interface>;
    using result_callback = std::function<void(bool)>;

    static constexpr const char* name = "commands";

    virtual ~commands_interface() = default;

    /// @brief Arm the vehicle (request).
    virtual command_handle::shared_ptr arm(result_callback&& cb) = 0;

    /// @brief Disarm the vehicle (request).
    virtual command_handle::shared_ptr disarm(result_callback&& cb) = 0;

    /// @brief Switch the flight mode (request).
    virtual command_handle::shared_ptr set_mode(data::flight_mode mode, result_callback&& cb) = 0;

    /// @brief Trigger an automatic takeoff (task).
    virtual command_handle::shared_ptr takeoff(result_callback&& cb) = 0;

    /// @brief Trigger an automatic landing (task).
    virtual command_handle::shared_ptr land(result_callback&& cb) = 0;

    /// @brief Trigger a return-to-launch (task).
    virtual command_handle::shared_ptr rtl(result_callback&& cb) = 0;
};

}  // namespace clover2_fcu::capability
