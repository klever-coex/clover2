#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_http_plugins/data/node_info.hpp>

// STL
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace clover2_http_plugins::utils {

class node_info_storage {
    using node_info = clover2_http_plugins::data::node_info;

public:
    explicit node_info_storage() = default;
    ~node_info_storage() = default;

    void update(std::shared_ptr<clover2_common::node_context> ctx);

    std::vector<std::string> names() const;
    bool has_node(const std::string& full_name) const;
    node_info operator[](const std::string& full_name) const;

private:
    mutable std::mutex m_mtx;
    std::unordered_map<std::string, node_info> m_nodes;
};

}  // namespace clover2_http_plugins::utils
