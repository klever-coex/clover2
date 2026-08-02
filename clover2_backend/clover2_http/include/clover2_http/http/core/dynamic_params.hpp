#pragma once

#include <clover2_http/http/core/exceptions.hpp>

#include <any>
#include <format>
#include <string>
#include <unordered_map>

namespace clover2_http::http::core {

class dynamic_params {
public:
    template <typename T>
    void set(const std::string& key, T value) {
        m_values[key] = std::move(value);
    }

    template <typename T>
    T at(const std::string& key) const {
        try {
            return std::any_cast<T>(m_values.at(key));
        } catch (const std::bad_any_cast&) {
            throw parsing_error("Unable cat {} to type {}", key,
                                typeid(T).name());
        }
    }

public:
    std::unordered_map<std::string, std::any> m_values;
};

}  // namespace clover2_http::http::core
