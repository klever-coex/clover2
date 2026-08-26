// clover2
#include <clover2_http_plugins/utils/node_info_storage.hpp>

// STL
#include <format>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace clover2_http_plugins::utils {

node_info_storage::node_info_storage(
    std::shared_ptr<clover2_common::node_context> ctx)
    : m_ctx(std::move(ctx)) {}

void node_info_storage::update() {
    std::lock_guard lock(m_mtx);

    const auto graph = m_ctx->get_node_graph_interface();
    auto node_names = graph->get_node_names_and_namespaces();

    std::unordered_set<std::string> present;
    present.reserve(node_names.size());

    std::unordered_map<std::string, std::shared_ptr<detail::node_client>> nodes;
    for (const auto& [name, ns] : node_names) {
        const auto full_name = data::full_node_name(ns, name);
        present.emplace(full_name);

        if (auto it = m_nodes.find(full_name); it == m_nodes.end()) {
            nodes.emplace(full_name, std::make_shared<detail::node_client>(
                                         m_ctx, name, ns));
        }
    }

    for (auto it = m_nodes.begin(); it != m_nodes.end();) {
        if (!present.contains(it->first)) {
            it = m_nodes.erase(it);
        } else {
            ++it;
        }
    }

    m_nodes.merge(nodes);
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

node_info_storage::node_info node_info_storage::get_info(
    const std::string& full_name) const {
    return find_client(full_name)->get_node_info();
}

std::vector<node_info_storage::topic_endpoint>
node_info_storage::get_publishers(const std::string& full_name) const {
    return find_client(full_name)->get_publishers();
}

std::vector<node_info_storage::topic_endpoint>
node_info_storage::get_subscribes(const std::string& full_name) const {
    return find_client(full_name)->get_subscribes();
}

std::vector<node_info_storage::service_endpoint> node_info_storage::get_servers(
    const std::string& full_name) const {
    return find_client(full_name)->get_servers();
}

std::vector<node_info_storage::service_endpoint> node_info_storage::get_clients(
    const std::string& full_name) const {
    return find_client(full_name)->get_clients();
}

std::shared_ptr<detail::node_client> node_info_storage::find_client(
    const std::string& full_name) const {
    std::lock_guard lock(m_mtx);

    const auto it = m_nodes.find(full_name);
    if (it == m_nodes.end()) {
        throw std::invalid_argument(std::format("Unknown node {}", full_name));
    }

    return it->second;
}

}  // namespace clover2_http_plugins::utils
