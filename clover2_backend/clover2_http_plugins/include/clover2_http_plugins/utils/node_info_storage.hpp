#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_http_plugins/data/node_info.hpp>
#include <clover2_http_plugins/data/service_endpoint.hpp>
#include <clover2_http_plugins/data/topic_endpoint.hpp>

// STL
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_http_plugins::utils {

class node_info_storage {
    using node_info = clover2_http_plugins::data::node_info;
    using service_endpoint = clover2_http_plugins::data::service_endpoint;
    using topic_endpoint = clover2_http_plugins::data::topic_endpoint;

public:
    explicit node_info_storage() = default;
    ~node_info_storage() = default;

    void update(std::shared_ptr<clover2_common::node_context> ctx);

    std::vector<std::string> names() const;
    bool has_node(const std::string& full_name) const;
    node_info operator[](const std::string& full_name) const;

    std::vector<topic_endpoint> get_publishers(
        const std::string& full_name) const;
    std::vector<topic_endpoint> get_subscribes(
        const std::string& full_name) const;
    std::vector<service_endpoint> get_servers(
        const std::string& full_name) const;
    std::vector<service_endpoint> get_clients(
        const std::string& full_name) const;

private:
    std::vector<topic_endpoint> endpoints(const std::string& full_name,
                                          bool publishers) const;
    std::vector<service_endpoint> service_endpoints(
        const std::string& full_name, bool servers) const;

    mutable std::mutex m_mtx;
    std::unordered_map<std::string, node_info> m_nodes;
    std::shared_ptr<clover2_common::node_context> m_ctx;
};

}  // namespace clover2_http_plugins::utils
