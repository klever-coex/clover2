#pragma once

#include <clover2_common/>
#include <clover2_http/server.hpp>

#include <memory>

namespace clover2_http_bridge {

class base_plugin {
public:
    using SharedPtr = std::shared_ptr<base_plugin>;
    virtual ~base_plugin() = default;

    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    std::shared_ptr<http::server> server);

protected:
    virtual void on_initialize();

    std::shared_ptr<http::server> m_server;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2_http_bridge
