// clover2
#include <clover2_http/plugins/data/node_info.hpp>

namespace clover2_http::plugins::data {

std::string full_node_name(const std::string& ns, const std::string& name) {
    return (ns == "/") ? ns + name : ns + "/" + name;
}

void to_json(nlohmann::json& json, const node_info& info) {
    json["name"] = info.name;
    json["ns"] = info.ns;
    json["is_lifecycle"] = info.is_lifecycle;

    if (info.lifecycle_state.has_value()) {
        json["lifecycle_state"] = info.lifecycle_state.value();
    }
}

void from_json(const nlohmann::json& json, node_info& info) {
    json.at("name").get_to(info.name);
    json.at("ns").get_to(info.ns);
    json.at("is_lifecycle").get_to(info.is_lifecycle);

    if (json.contains("lifecycle_state")) {
        info.lifecycle_state =
            json.at("lifecycle_state").get<lifecycle_state>();
    }
}

}  // namespace clover2_http::plugins::data
