#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_http/plugins/data/node_info.hpp>
#include <clover2_http/plugins/data/service_endpoint.hpp>
#include <clover2_http/plugins/data/topic_endpoint.hpp>
#include <clover2_http/plugins/utils/detail/node_client.hpp>

// STL
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_http::plugins::utils {

class node_info_storage {
    using node_info = clover2_http::plugins::data::node_info;
    using service_endpoint = clover2_http::plugins::data::service_endpoint;
    using topic_endpoint = clover2_http::plugins::data::topic_endpoint;

public:
    explicit node_info_storage(
        std::shared_ptr<clover2_common::node_context> ctx);
    ~node_info_storage() = default;

    void update();

    std::vector<std::string> names() const;
    bool has_node(const std::string& full_name) const;
    node_info get_info(const std::string& full_name) const;

    std::vector<topic_endpoint> get_publishers(
        const std::string& full_name) const;
    std::vector<topic_endpoint> get_subscribes(
        const std::string& full_name) const;
    std::vector<service_endpoint> get_servers(
        const std::string& full_name) const;
    std::vector<service_endpoint> get_clients(
        const std::string& full_name) const;

private:
    std::shared_ptr<detail::node_client> find_client(
        const std::string& full_name) const;

    mutable std::mutex m_mtx;
    std::shared_ptr<clover2_common::node_context> m_ctx;
    std::unordered_map<std::string, std::shared_ptr<detail::node_client>>
        m_nodes;
};

}  // namespace clover2_http::plugins::utils
