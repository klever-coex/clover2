#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace clover2::backend::model {

template <typename T>
struct base_model {
    virtual ~base_model() = default;

    std::string serialize() const {
        nlohmann::json j = *static_cast<const T*>(this);
        return j.dump();
    }

    void deserialize(const std::string& data) {
        nlohmann::json j = nlohmann::json::parse(data);
        *static_cast<T*>(this) = j.get<T>();
    }
};

}  // namespace clover2::backend::model
