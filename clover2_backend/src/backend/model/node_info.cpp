#include <clover2/backend/model/node_info.hpp>
#include <nlohmann/json.hpp>

namespace clover2::backend::model {

node_info::node_info()
    : m_id("")
    , m_name("")
    , m_namespace("")
    , m_type(data::node_type::type::simple_node)
    , m_publishing_topics()
    , m_subscribing_topics() {}

const std::string& node_info::get_id() const { return m_id; }
void node_info::set_id(const std::string& value) { m_id = value; }

const std::string& node_info::get_name() const { return m_name; }
void node_info::set_name(const std::string& value) { m_name = value; }

const std::string& node_info::get_namespace() const { return m_namespace; }
void node_info::set_namespace(const std::string& value) { m_namespace = value; }

data::node_type node_info::get_type() const { return m_type; }
void node_info::set_type(const data::node_type& value) { m_type = value; }

const std::vector<std::string>& node_info::get_publishing_topics() const {
    return m_publishing_topics;
}
void node_info::set_publishing_topics(const std::vector<std::string>& value) {
    m_publishing_topics = value;
}

const std::vector<std::string>& node_info::get_subscribing_topics() const {
    return m_subscribing_topics;
}
void node_info::set_subscribing_topics(const std::vector<std::string>& value) {
    m_subscribing_topics = value;
}

void to_json(nlohmann::json& j, const node_info& p) {
    j["id"] = p.get_id();
    j["name"] = p.get_name();
    j["namespace"] = p.get_namespace();
    j["type"] = data::node_type::to_string(p.get_type());
    j["publishing_topics"] = p.get_publishing_topics();
    j["subscribing_topics"] = p.get_subscribing_topics();
}

void from_json(const nlohmann::json& j, node_info& p) {
    p.set_id(j.at("id").get<std::string>());
    p.set_name(j.at("name").get<std::string>());
    p.set_namespace(j.at("namespace").get<std::string>());
    p.set_type(data::node_type::from_string(j.at("type").get<std::string>()));
    if (j.contains("publishing_topics")) {
        p.set_publishing_topics(
            j.at("publishing_topics").get<std::vector<std::string>>());
    }
    if (j.contains("subscribing_topics")) {
        p.set_subscribing_topics(
            j.at("subscribing_topics").get<std::vector<std::string>>());
    }
}

}  // namespace clover2::backend::model
