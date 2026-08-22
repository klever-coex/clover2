#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_http/http/server.hpp>
#include <clover2_http/data/plugin_info.hpp>

// ROS2
#include <rclcpp/logger.hpp>

// STL
#include <memory>
#include <string>
#include <vector>

namespace clover2_http {

class base_plugin {
public:
    using SharedPtr = std::shared_ptr<base_plugin>;

    base_plugin()
        : m_logger(rclcpp::get_logger("http.base_plugin")) {}

    virtual ~base_plugin() = default;

    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    std::shared_ptr<clover2_http::http::server> server) {
        m_node_context = std::move(node_context);
        m_server = std::move(server);
        m_logger = m_node_context->get_node_logging_interface()
                       ->get_logger()
                       .get_child(name());
        on_initialize();
    }

    virtual std::string name() const { return {}; }
    virtual int version() const { return 0; }
    virtual std::vector<std::string> capabilities() const { return {}; }

    data::plugin_info manifest() const {
        return {name(), version(), capabilities()};
    }

    rclcpp::Logger get_logger() const { return m_logger; }

protected:
    virtual void on_initialize() = 0;

    rclcpp::Logger m_logger;
    std::shared_ptr<clover2_http::http::server> m_server;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2_http
