#include <clover2/backend/model/action_info.hpp>
#include <nlohmann/json.hpp>

namespace clover2::backend::model {

action_info::action_info()
    : m_id("")
    , m_name("")
    , m_type("") {}

const std::string& action_info::get_id() const { return m_id; }
void action_info::set_id(const std::string& value) { m_id = value; }

const std::string& action_info::get_name() const { return m_name; }
void action_info::set_name(const std::string& value) { m_name = value; }

const std::string& action_info::get_type() const { return m_type; }
void action_info::set_type(const std::string& value) { m_type = value; }

void to_json(nlohmann::json& j, const action_info& p) {
    j["id"] = p.get_id();
    j["name"] = p.get_name();
    j["type"] = p.get_type();
}

void from_json(const nlohmann::json& j, action_info& p) {
    p.set_id(j.at("id").get<std::string>());
    p.set_name(j.at("name").get<std::string>());
    p.set_type(j.at("type").get<std::string>());
}

}  // namespace clover2::backend::model

