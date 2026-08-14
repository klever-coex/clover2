#pragma once

#include <clover2_common/node.hpp>
#include <clover2_http/base_plugin.hpp>

#include <boost/asio/io_context.hpp>
#include <pluginlib/class_loader.hpp>

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

namespace clover2_http::server {

class node : public clover2_common::node {
public:
    explicit node(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~node() override;

private:
    std::shared_ptr<boost::asio::io_context> m_io;
    std::shared_ptr<clover2_http::http::server> m_server;
    std::thread m_io_thread;

    pluginlib::ClassLoader<base_plugin> m_plugin_loader;
    std::unordered_map<std::string, base_plugin::SharedPtr> m_plugins;
};

}  // namespace clover2_http::server
