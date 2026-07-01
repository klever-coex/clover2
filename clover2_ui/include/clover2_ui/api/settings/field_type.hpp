#pragma once

#include <stdexcept>

namespace clover2_ui::api::settings {

class field_type {
public:
    enum value {
        STR,
        BOOL,
        INT,
        FLOAT,
        OBJECT,
    };

    field_type() = default;
    constexpr field_type(value v)
        : m_value(v) {}

    field_type(std::string_view str) { m_value = from_string(str); }

    constexpr operator value() const noexcept { return m_value; }

    constexpr field_type& operator=(value v) noexcept {
        m_value = v;
        return *this;
    }

    constexpr bool operator==(value v) const noexcept { return m_value == v; }

    constexpr bool operator!=(value v) const noexcept { return m_value != v; }

    constexpr bool operator==(field_type other) const noexcept {
        return m_value == other.m_value;
    }

    constexpr bool operator!=(field_type other) const noexcept {
        return m_value != other.m_value;
    }

    constexpr std::string_view to_string() const noexcept {
        return to_string(m_value);
    }

    static constexpr std::string_view to_string(value v) {
        switch (v) {
            case STR:
                return "str";
            case BOOL:
                return "bool";
            case INT:
                return "int";
            case FLOAT:
                return "float";
            case OBJECT:
                return "object";
        }

        throw std::invalid_argument("Unknown field_type");
    }

    static value from_string(std::string_view str) {
        if (str == "str") return STR;
        if (str == "bool") return BOOL;
        if (str == "int") return INT;
        if (str == "float") return FLOAT;
        if (str == "object") return OBJECT;

        throw std::invalid_argument("Unknown field_type");
    }

private:
    value m_value = value::STR;
};

}  // namespace clover2_ui::api::settings
