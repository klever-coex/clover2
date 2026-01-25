#include <clover2/backend/model/topic_info.hpp>
#include <nlohmann/json.hpp>

namespace clover2::backend::model {

topic_info::topic_info()
    : m_id("")
    , m_name("")
    , m_type("")
    , m_qos_profile(nlohmann::json::object()) {}

const std::string& topic_info::get_id() const { return m_id; }
void topic_info::set_id(const std::string& value) { m_id = value; }

const std::string& topic_info::get_name() const { return m_name; }
void topic_info::set_name(const std::string& value) { m_name = value; }

const std::string& topic_info::get_type() const { return m_type; }
void topic_info::set_type(const std::string& value) { m_type = value; }

const nlohmann::json& topic_info::get_qos_profile() const { return m_qos_profile; }
void topic_info::set_qos_profile(const nlohmann::json& value) { m_qos_profile = value; }

void to_json(nlohmann::json& j, const topic_info& p) {
    j["id"] = p.get_id();
    j["name"] = p.get_name();
    j["type"] = p.get_type();
    j["qos_profile"] = p.get_qos_profile();
}

void from_json(const nlohmann::json& j, topic_info& p) {
    p.set_id(j.at("id").get<std::string>());
    p.set_name(j.at("name").get<std::string>());
    p.set_type(j.at("type").get<std::string>());
    if (j.contains("qos_profile")) {
        p.set_qos_profile(j.at("qos_profile"));
    } else {
        p.set_qos_profile(nlohmann::json::object());
    }
}

}  // namespace clover2::backend::model
