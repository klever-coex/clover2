#pragma once

// clover2
#include <clover2_http_plugins/data/marker_pose.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <optional>
#include <string>

namespace clover2_http_plugins::data {

struct marker_info {
    int id = 0;
    std::string type = "fixed";
    double size = -1.0;
    std::string marker_frame_id;
    std::optional<marker_pose> pose;
};

inline void to_json(nlohmann::json& j, const marker_info& m) {
    j = nlohmann::json{{"id", m.id},
                       {"type", m.type},
                       {"size", m.size},
                       {"marker_frame_id", m.marker_frame_id}};
    if (m.pose) {
        j["pose"] = *m.pose;
    }
}

inline void from_json(const nlohmann::json& j, marker_info& m) {
    j.at("id").get_to(m.id);
    j.at("type").get_to(m.type);

    if (j.contains("size")) {
        j.at("size").get_to(m.size);
    }

    if (j.contains("marker_frame_id")) {
        j.at("marker_frame_id").get_to(m.marker_frame_id);
    }

    if (j.contains("pose") && !j.at("pose").is_null()) {
        m.pose = j.at("pose").get<marker_pose>();
    }
}

}  // namespace clover2_http_plugins::data
