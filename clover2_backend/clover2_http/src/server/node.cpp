#include <clover2_http/server/data/manifest.hpp>
#include <clover2_http/server/logger.hpp>
#include <clover2_http/server/node.hpp>

namespace clover2_http::server {

node::node(const rclcpp::NodeOptions& options)
    : clover2_common::node("http", options)
    , m_io(std::make_shared<boost::asio::io_context>())
    , m_server(std::make_shared<clover2_http::http::server>(
          *m_io, logger(get_logger().get_child("server"))))
    , m_plugin_loader("clover2_http_plugins", "clover2_http::base_plugin") {
    const auto address =
        declare_parameter<std::string>("http_address", "0.0.0.0");
    const auto port = declare_parameter<int64_t>("http_port", 8080);

    auto manifest = std::make_shared<server::data::manifest>();
    m_server->get<void, server::data::manifest>(
        "/manifest",
        [manifest](
            clover2_http::http::core::request_context,
            clover2_http::http::endpoint::reply<server::data::manifest> reply) {
            reply(*manifest, 200);
        });

    auto node_context = std::make_shared<clover2_common::node_context>(*this);

    for (const auto& plugin_class : m_plugin_loader.getDeclaredClasses()) {
        auto plugin = m_plugin_loader.createUniqueInstance(plugin_class);
        RCLCPP_INFO(get_logger(), "Loading http plugin: %s",
                    plugin_class.c_str());
        plugin->initialize(node_context, m_server);
        auto info = plugin->manifest();
        if (info.name.empty()) {
            info.name = plugin_class;
        }
        manifest->plugins.push_back(std::move(info));
        m_plugins[plugin_class] = std::move(plugin);
    }

    m_server->listen(address, static_cast<uint16_t>(port));

    m_io_thread = std::thread([io = m_io]() { io->run(); });
}

node::~node() {
    m_server->stop();
    m_io->stop();
    if (m_io_thread.joinable()) {
        m_io_thread.join();
    }
}

}  // namespace clover2_http::server

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_http::server::node)
