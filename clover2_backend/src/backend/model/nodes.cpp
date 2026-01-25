#include <clover2/backend/model/nodes.hpp>
#include <nlohmann/json.hpp>

namespace clover2::backend::model {

nodes::nodes() : m_nodes() {}

const std::vector<node_info>& nodes::get_nodes() const { return m_nodes; }
void nodes::set_nodes(const std::vector<node_info>& nodes) { m_nodes = nodes; }

void to_json(nlohmann::json& j, const nodes& p) { j["nodes"] = p.get_nodes(); }

void from_json(const nlohmann::json& j, nodes& p) {
    p.set_nodes(j.at("nodes").get<std::vector<node_info>>());
}

}  // namespace clover2::backend::model
