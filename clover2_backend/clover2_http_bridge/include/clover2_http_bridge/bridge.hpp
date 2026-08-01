#pragma once

#include <clover2_common/node.hpp>
#include <clover2_http/server.hpp>

namespace clover2_http_bridge {

class bridge : public clover2_common::node {
public:
    explicit bridge(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    std::shared_ptr<clover2_http::server> m_server;

    pluginlib::ClassLoader<base_plugin> m_plugin_loader{
        "clover2_http_bridge", "clover2_http_bridge::base_plugin"};
    std::unordered_map<std::string, base_plugin::SharedPtr> m_plugins;
};

}  // namespace clover2_http_bridge
