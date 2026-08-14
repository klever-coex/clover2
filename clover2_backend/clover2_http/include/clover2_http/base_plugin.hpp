#pragma once

#include <clover2_common/node_context.hpp>
#include <clover2_http/http/server.hpp>
#include <clover2_http/server/data/plugin_info.hpp>

#include <memory>
#include <string>
#include <vector>

namespace clover2_http {

class base_plugin {
public:
    using SharedPtr = std::shared_ptr<base_plugin>;
    virtual ~base_plugin() = default;

    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    std::shared_ptr<clover2_http::http::server> server) {
        m_node_context = std::move(node_context);
        m_server = std::move(server);
        on_initialize();
    }

    virtual std::string name() const { return {}; }
    virtual int version() const { return 0; }
    virtual std::vector<std::string> capabilities() const { return {}; }

    server::data::plugin_info manifest() const {
        return {name(), version(), capabilities()};
    }

protected:
    virtual void on_initialize() = 0;

    std::shared_ptr<clover2_http::http::server> m_server;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2_http
