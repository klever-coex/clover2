#pragma once

// clover2
#include <clover2_fcu/common/result.hpp>

// STL
#include <memory>
#include <string>
#include <variant>

namespace clover2_fcu::capability {

using parameter_value = std::variant<double, int64_t, bool, std::string>;

class parameters_interface {
public:
    using shared_ptr = std::shared_ptr<parameters_interface>;

    static constexpr const char* name = "parameters";

    virtual ~parameters_interface() = default;

    virtual result get(const std::string& name, parameter_value& out) = 0;

    virtual result set(const std::string& name,
                       const parameter_value& value) = 0;
};

}  // namespace clover2_fcu::capability
