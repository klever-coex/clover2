#pragma once

// clover2
#include <clover2_fcu/common/result.hpp>

#include <rclcpp/parameter_value.hpp>

// STL
#include <memory>
#include <string>
#include <vector>

namespace clover2_fcu::capability {

class parameters_interface {
public:
    using shared_ptr = std::shared_ptr<parameters_interface>;

    static constexpr const char* name = "parameters";

    virtual ~parameters_interface() = default;

    /// @brief Read a parameter from the provider's cache.
    virtual result get(const std::string& name,
                       rclcpp::ParameterValue& out) = 0;

    /// @brief Write a parameter (blocking round-trip); must not be called
    ///        from inside an fcu-group callback.
    virtual result set(const std::string& name,
                       const rclcpp::ParameterValue& value) = 0;

    virtual std::vector<std::string> list() const = 0;
};

}  // namespace clover2_fcu::capability
