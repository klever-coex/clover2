#pragma once

#include <clover2_http/plugin.hpp>
#include <clover2_http_plugins/data/node_info.hpp>
#include <clover2_http_plugins/data/nodes.hpp>
#include <clover2_http_plugins/data/topic_info.hpp>
#include <clover2_http_plugins/data/topics.hpp>
#include <clover2_http_plugins/utils/graph_listener.hpp>
#include <clover2_http_plugins/utils/node_info_storage.hpp>

namespace clover2_http_plugins {

class ros_support : public clover2_http::plugin<ros_support> {
public:
    static constexpr std::string_view k_name = "ros_support";
    static constexpr int k_version = 1;

protected:
    void on_initialize() override;
    std::vector<std::string> capabilities() const override;

private:
    void handle_nodes(
        clover2_http::http::core::request_context ctx,
        clover2_http::http::endpoint::reply<clover2_http_plugins::data::nodes>
            reply);

    void handle_node_info(clover2_http::http::core::request_context ctx,
                          clover2_http::http::endpoint::reply<
                              clover2_http_plugins::data::node_info>
                              reply);

    void handle_topics(
        clover2_http::http::core::request_context ctx,
        clover2_http::http::endpoint::reply<clover2_http_plugins::data::topics>
            reply);

    void handle_topic_json_stream(
        std::shared_ptr<clover2_http::http::transport::base_ws_session>
            session);

    double m_rate_limit = 100000.0;
    clover2_http_plugins::utils::node_info_storage m_node_info_storage;
    std::shared_ptr<clover2_http_plugins::utils::graph_listener>
        m_graph_listener;
};

}  // namespace clover2_http_plugins
