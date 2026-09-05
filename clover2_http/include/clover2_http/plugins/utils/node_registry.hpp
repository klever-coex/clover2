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

class node_registry {
    using node_info = clover2_http::plugins::data::node_info;
    using service_endpoint = clover2_http::plugins::data::service_endpoint;
    using topic_endpoint = clover2_http::plugins::data::topic_endpoint;

    using transition_cb = detail::node_client::transition_cb;
    using available_transitions_cb =
        detail::node_client::available_transitions_cb;

public:
    explicit node_registry(
        std::shared_ptr<clover2_common::node_context> ctx);
    ~node_registry() = default;

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

    void transition(const std::string& full_name,
                    const data::lifecycle_transition& transition,
                    transition_cb&& cb);
    void get_available_transitions(const std::string& full_name,
                                   available_transitions_cb&& cb);

private:
    std::shared_ptr<detail::node_client> find_client(
        const std::string& full_name) const;

    mutable std::mutex m_mtx;
    std::shared_ptr<clover2_common::node_context> m_ctx;
    std::unordered_map<std::string, std::shared_ptr<detail::node_client>>
        m_nodes;
};

}  // namespace clover2_http::plugins::utils
