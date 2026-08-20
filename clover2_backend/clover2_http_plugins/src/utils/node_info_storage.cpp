// clover2
#include <clover2_http_plugins/utils/node_info_storage.hpp>

// STL
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace clover2_http_plugins::utils {

void node_info_storage::update(
    std::shared_ptr<clover2_common::node_context> ctx) {
    std::unordered_map<std::string, node_info> nodes;

    const auto graph = ctx->get_node_graph_interface();
    const auto services = graph->get_service_names_and_types();

    for (const auto& [name, ns] : graph->get_node_names_and_namespaces()) {
        node_info info;
        info.name = name;
        info.ns = ns;

        const auto full_name = data::full_node_name(ns, name);
        info.is_lifecycle = services.contains(full_name + "/change_state");

        nodes.emplace(full_name, std::move(info));
    }

    std::lock_guard lock(m_mtx);
    m_ctx = std::move(ctx);
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

std::vector<node_info_storage::topic_endpoint>
node_info_storage::get_publishers(const std::string& full_name) const {
    return endpoints(full_name, true);
}

std::vector<node_info_storage::topic_endpoint>
node_info_storage::get_subscribes(const std::string& full_name) const {
    return endpoints(full_name, false);
}

std::vector<node_info_storage::topic_endpoint> node_info_storage::endpoints(
    const std::string& full_name, bool publishers) const {
    const auto graph = m_ctx->get_node_graph_interface();

    const auto node_info = (*this)[full_name];
    const auto& name = node_info.name;
    const auto& ns = node_info.ns;

    const auto names_and_types =
        publishers ? graph->get_publisher_names_and_types_by_node(name, ns)
                   : graph->get_subscriber_names_and_types_by_node(name, ns);

    std::vector<topic_endpoint> result;
    for (const auto& [topic, types] : names_and_types) {
        topic_endpoint endpoint;
        endpoint.info.name = topic;
        endpoint.info.type = types.empty() ? std::string{} : types.front();

        const auto infos = publishers
                               ? graph->get_publishers_info_by_topic(topic)
                               : graph->get_subscriptions_info_by_topic(topic);

        for (const auto& info : infos) {
            if (data::full_node_name(info.node_namespace(), info.node_name()) ==
                full_name) {
                endpoint.info.type = info.topic_type();
                endpoint.qos_profile = data::to_qos(info.qos_profile());
                break;
            }
        }

        result.push_back(std::move(endpoint));
    }

    return result;
}

std::vector<node_info_storage::service_endpoint>
node_info_storage::get_servers(const std::string& full_name) const {
    return service_endpoints(full_name, true);
}

std::vector<node_info_storage::service_endpoint>
node_info_storage::get_clients(const std::string& full_name) const {
    return service_endpoints(full_name, false);
}

std::vector<node_info_storage::service_endpoint>
node_info_storage::service_endpoints(const std::string& full_name,
                                     bool servers) const {
    const auto graph = m_ctx->get_node_graph_interface();

    const auto node_info = (*this)[full_name];
    const auto& name = node_info.name;
    const auto& ns = node_info.ns;

    const auto names_and_types =
        servers ? graph->get_service_names_and_types_by_node(name, ns)
                : graph->get_client_names_and_types_by_node(name, ns);

    std::vector<service_endpoint> result;
    for (const auto& [service, types] : names_and_types) {
        service_endpoint endpoint;
        endpoint.info.name = service;
        endpoint.info.type = types.empty() ? std::string{} : types.front();

        result.push_back(std::move(endpoint));
    }

    return result;
}

}  // namespace clover2_http_plugins::utils
