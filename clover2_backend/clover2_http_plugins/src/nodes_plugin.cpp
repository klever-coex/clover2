#include <clover2_http/plugin.hpp>
#include <clover2_http_plugins/data/topic_ref.hpp>

#include <pluginlib/class_list_macros.hpp>
#include <rclcpp/graph_listener.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace clover2_http_plugins {

using data::topic_ref;

struct node_info {
    std::string name;
    std::vector<topic_ref> publishers;
    std::vector<topic_ref> subscribers;
    std::vector<topic_ref> services;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(node_info, name, publishers, subscribers,
                                   services)

struct nodes_response {
    std::vector<node_info> nodes;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(nodes_response, nodes)

template <typename MapT>
std::vector<topic_ref> to_refs(const MapT& names_and_types) {
    std::vector<topic_ref> refs;
    for (const auto& [name, types] : names_and_types) {
        refs.push_back({name, types.empty() ? std::string{} : types.front()});
    }

    return refs;
}

class nodes_plugin : public clover2_http::plugin<nodes_plugin> {
public:
    static constexpr std::string_view k_name = "nodes";
    static constexpr int k_version = 1;

protected:
    void on_initialize() override {
        m_server->get<void, nodes_response>(
            "/nodes",
            [node_context = m_node_context](
                clover2_http::http::core::request_context,
                clover2_http::http::endpoint::reply<nodes_response> reply) {
                nodes_response response;

                auto graph = node_context->get_node_graph_interface();
                for (const auto& [name, ns] :
                     graph->get_node_names_and_namespaces()) {
                    node_info info;
                    info.name = (ns == "/") ? ns + name : ns + "/" + name;

                    try {
                        info.publishers = to_refs(
                            graph->get_publisher_names_and_types_by_node(name,
                                                                         ns));
                        info.subscribers = to_refs(
                            graph->get_subscriber_names_and_types_by_node(name,
                                                                          ns));
                        info.services =
                            to_refs(graph->get_service_names_and_types_by_node(
                                name, ns));
                    } catch (const std::exception&) {
                        continue;
                    }

                    response.nodes.push_back(std::move(info));
                }

                reply(response, 200);
            });
    }

    std::vector<std::string> capabilities() const override { return {"nodes"}; }
};

}  // namespace clover2_http_plugins

PLUGINLIB_EXPORT_CLASS(clover2_http_plugins::nodes_plugin,
                       clover2_http::base_plugin)
