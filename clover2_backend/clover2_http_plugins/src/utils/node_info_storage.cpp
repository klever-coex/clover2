// clover2
#include <clover2_http_plugins/utils/node_info_storage.hpp>

// STL
#include <exception>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace clover2_http_plugins::utils {

namespace {

using data::node_info;
using data::topic_info;

std::vector<topic_info> to_topic_infos(
    const std::map<std::string, std::vector<std::string>>& names_and_types) {
    std::vector<topic_info> infos;
    infos.reserve(names_and_types.size());
    for (const auto& [name, types] : names_and_types) {
        infos.push_back({name, types.empty() ? std::string{} : types.front()});
    }
    return infos;
}

}  // namespace

void node_info_storage::update(
    std::shared_ptr<clover2_common::node_context> ctx) {
    std::unordered_map<std::string, node_info> nodes;

    const auto graph = ctx->get_node_graph_interface();
    for (const auto& [name, ns] : graph->get_node_names_and_namespaces()) {
        node_info info;
        info.name = name;
        info.ns = ns;

        try {
            info.publishers = to_topic_infos(
                graph->get_publisher_names_and_types_by_node(name, ns));
            info.subscribers = to_topic_infos(
                graph->get_subscriber_names_and_types_by_node(name, ns));
        } catch (const std::exception&) {
            continue;
        }

        nodes.emplace(data::full_node_name(ns, name), std::move(info));
    }

    std::lock_guard lock(m_mtx);
    m_nodes = std::move(nodes);
}

std::vector<std::string> node_info_storage::names() const {
    std::lock_guard lock(m_mtx);

    std::vector<std::string> names;
    names.reserve(m_nodes.size());

    for (const auto& [name, _] : m_nodes) {
        names.push_back(name);
    }

    return names;
}

bool node_info_storage::has_node(const std::string& full_name) const {
    std::lock_guard lock(m_mtx);
    return m_nodes.contains(full_name);
}

node_info_storage::node_info node_info_storage::operator[](
    const std::string& full_name) const {
    std::lock_guard lock(m_mtx);

    const auto it = m_nodes.find(full_name);
    if (it == m_nodes.end()) {
        throw std::invalid_argument(std::format("Unknown node {}", full_name));
    }

    return it->second;
}

}  // namespace clover2_http_plugins::utils
