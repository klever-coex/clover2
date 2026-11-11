#pragma once

// STL
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace clover2_fcu {

class result {
public:
    enum class value : uint8_t {
        success = 0,
        rejected,
        timeout,
        interrupted,
        unsupported,
    };

    result() noexcept = default;
    result(value v) noexcept
        : m_value(v) {}

    operator value() const noexcept { return m_value; }
    std::string_view to_string() const noexcept;

    static std::optional<result> from_string(const std::string& name);

private:
    value m_value{value::success};
};

}  // namespace clover2_fcu
