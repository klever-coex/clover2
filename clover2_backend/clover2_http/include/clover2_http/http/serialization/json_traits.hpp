#pragma once

#include <nlohmann/json.hpp>

#include <vector>

namespace clover2_http::http::serialization {

template <typename T>
struct json_traits {
    static T from_json(const nlohmann::json& jv) = delete;
    static void to_json(nlohmann::json& jv, const T& obj) = delete;
};

template <typename T>
struct json_traits<std::vector<T>> {
    static std::vector<T> from_json(const nlohmann::json& jv) {
        std::vector<T> result;
        result.reserve(jv.size());
        for (auto const& elem : jv) {
            result.push_back(json_traits<T>::from_json(elem));
        }
        return result;
    }

    static void to_json(nlohmann::json& jv, const std::vector<T>& vec) {
        jv = nlohmann::json::array();
        for (auto const& elem : vec) {
            nlohmann::json elem_jv;
            json_traits<T>::to_json(elem_jv, elem);
            jv.push_back(std::move(elem_jv));
        }
    }
};

template <>
struct json_traits<nlohmann::json> {
    static nlohmann::json from_json(const nlohmann::json& jv) { return jv; }
    static void to_json(nlohmann::json& jv, const nlohmann::json& obj) {
        jv = obj;
    }
};

namespace impl {

template <typename T>
inline T deserialize(const nlohmann::json& jv) {
    return json_traits<T>::from_json(jv);
}

template <>
inline void deserialize<void>(const nlohmann::json&) {}

template <typename T>
inline nlohmann::json serialize(const T& obj) {
    nlohmann::json jv;
    json_traits<T>::to_json(jv, obj);
    return jv;
}

}  // namespace impl
}  // namespace clover2_http::http::serialization
