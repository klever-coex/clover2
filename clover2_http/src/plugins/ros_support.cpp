// clover2
#include <clover2_common/util/parameter.hpp>
#include <clover2_http/plugin.hpp>
#include <clover2_http/plugins/data/node_info.hpp>
#include <clover2_http/plugins/data/node_services.hpp>
#include <clover2_http/plugins/data/node_topics.hpp>
#include <clover2_http/plugins/data/nodes.hpp>
#include <clover2_http/plugins/data/topic_info.hpp>
#include <clover2_http/plugins/data/topics.hpp>
#include <clover2_http/plugins/utils/graph_listener.hpp>
#include <clover2_http/plugins/utils/message_type.hpp>
#include <clover2_http/plugins/utils/msg_json.hpp>
#include <clover2_http/plugins/utils/node_info_storage.hpp>
#include <clover2_http/plugins/utils/rate_limiter.hpp>
#include <clover2_http/plugins/utils/universal_subscriber.hpp>

// ROS2
#include <rclcpp/graph_listener.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_http::plugins {

using namespace clover2_http::plugins::data;

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
        clover2_http::http::endpoint::deferred_reply<data::nodes> reply);

    void handle_node_info(
        clover2_http::http::core::request_context ctx,
        clover2_http::http::endpoint::deferred_reply<data::node_info> reply);

    void handle_topics(
        clover2_http::http::core::request_context ctx,
        clover2_http::http::endpoint::deferred_reply<data::topics> reply);

    void handle_topic_json_stream(
        std::shared_ptr<clover2_http::http::transport::base_ws_session>
            session);

    double m_rate_limit = 100000.0;
    std::shared_ptr<clover2_http::plugins::utils::node_info_storage>
        m_node_info_storage;
    std::shared_ptr<clover2_http::plugins::utils::graph_listener>
        m_graph_listener;
};

namespace {

struct stream_state {
    stream_state(const std::string& type_name, double bytes_per_second)
        : type(type_name)
        , limiter(bytes_per_second) {}

    clover2_http::plugins::utils::message_type type;
    clover2_http::plugins::utils::rate_limiter limiter;
    std::unique_ptr<clover2_http::plugins::utils::universal_subscriber>
        subscriber;
};

template <typename MapT>
std::vector<topic_info> to_refs(const MapT& names_and_types) {
    std::vector<topic_info> refs;
    for (const auto& [name, types] : names_and_types) {
        refs.push_back({name, types.empty() ? std::string{} : types.front()});
    }

    return refs;
}

std::optional<nlohmann::json> deserialize_message(
    std::shared_ptr<rclcpp::SerializedMessage> serialized,
    const std::shared_ptr<stream_state>& state) {
    nlohmann::json j;

    try {
        if (!state->limiter.allow(serialized->size())) {
            return std::nullopt;
        }

        void* msg = state->type.allocate();
        if (!state->type.deserialize(*serialized, msg)) {
            state->type.deallocate(msg);
            return std::nullopt;
        }

        j = clover2_http::plugins::utils::msg_json::detail::to_json(
            state->type.members(), msg);

        state->type.deallocate(msg);
    } catch (const std::exception& e) {
        j = nlohmann::json{{"error", e.what()}};
    }

    return j;
}

}  // namespace

void ros_support::on_initialize() {
    auto parameters_watcher =
        m_node_context->get_node_parameters_watcher_interface();

    m_node_info_storage =
        std::make_shared<utils::node_info_storage>(m_node_context);

    m_graph_listener =
        std::make_shared<clover2_http::plugins::utils::graph_listener>(
            m_node_context, [this]() {
                RCLCPP_INFO(get_logger(), "Update graph signal");
                m_node_info_storage->update();
            });

    clover2_common::util::declare_and_watch_parameter<double>(
        parameters_watcher, "topics.rate_limit_bps", m_rate_limit,
        [this](const rclcpp::Parameter& p) { m_rate_limit = p.as_double(); },
        "Rate limit for topic streams in bytes per second");

    m_server->get<topics>(
        "/api/topics", std::bind(&ros_support::handle_topics, this,
                                 std::placeholders::_1, std::placeholders::_2));

    m_server->get<nodes>(
        "/api/nodes", std::bind(&ros_support::handle_nodes, this,
                                std::placeholders::_1, std::placeholders::_2));

    m_server->get<node_info>(
        "/api/node/info/-/{node...}",
        std::bind(&ros_support::handle_node_info, this, std::placeholders::_1,
                  std::placeholders::_2));

    m_server->get<node_topics>(
        "/api/node/publishers/-/{node...}",
        [this](
            clover2_http::http::core::request_context ctx,
            clover2_http::http::endpoint::deferred_reply<node_topics> reply) {
            std::string node_name = ctx.param<std::string>("node");
            if (node_name.empty() || node_name.front() != '/') {
                node_name.insert(node_name.begin(), '/');
            }

            node_topics response;
            response.topics = m_node_info_storage->get_publishers(node_name);

            reply(std::move(response), 200);
        });

    m_server->get<node_topics>(
        "/api/node/subscribes/-/{node...}",
        [this](
            clover2_http::http::core::request_context ctx,
            clover2_http::http::endpoint::deferred_reply<node_topics> reply) {
            std::string node_name = ctx.param<std::string>("node");
            if (node_name.empty() || node_name.front() != '/') {
                node_name.insert(node_name.begin(), '/');
            }

            node_topics response;
            response.topics = m_node_info_storage->get_subscribes(node_name);

            reply(std::move(response), 200);
        });

    m_server->get<node_services>(
        "/api/node/servers/-/{node...}",
        [this](
            clover2_http::http::core::request_context ctx,
            clover2_http::http::endpoint::deferred_reply<node_services> reply) {
            std::string node_name = ctx.param<std::string>("node");
            if (node_name.empty() || node_name.front() != '/') {
                node_name.insert(node_name.begin(), '/');
            }

            node_services response;
            response.services = m_node_info_storage->get_servers(node_name);

            reply(std::move(response), 200);
        });

    m_server->get<node_services>(
        "/api/node/clients/-/{node...}",
        [this](
            clover2_http::http::core::request_context ctx,
            clover2_http::http::endpoint::deferred_reply<node_services> reply) {
            std::string node_name = ctx.param<std::string>("node");
            if (node_name.empty() || node_name.front() != '/') {
                node_name.insert(node_name.begin(), '/');
            }

            node_services response;
            response.services = m_node_info_storage->get_clients(node_name);

            reply(std::move(response), 200);
        });

    m_server->raw_ws("/ws/topic/json/-/{topic...}",
                     std::bind(&ros_support::handle_topic_json_stream, this,
                               std::placeholders::_1));

    // TODO:
    // m_server->raw_ws("/topic/binary/-/{topic...}",
    //                  std::bind(&topics_plugin::handle_topic_binary_stream,
    //                  this,
    //                            std::placeholders::_1));
}

std::vector<std::string> ros_support::capabilities() const {
    return {"nodes", "topics", "services"};
}

void ros_support::handle_nodes(
    [[maybe_unused]] clover2_http::http::core::request_context ctx,
    clover2_http::http::endpoint::deferred_reply<nodes> reply) {
    nodes response;

    response.nodes = m_node_info_storage->names();

    reply(std::move(response), 200);
}

void ros_support::handle_node_info(
    [[maybe_unused]] clover2_http::http::core::request_context ctx,
    clover2_http::http::endpoint::deferred_reply<node_info> reply) {
    std::string node_name = ctx.param<std::string>("node");
    if (node_name.empty() || node_name.front() != '/') {
        node_name.insert(node_name.begin(), '/');
    }

    try {
        node_info response = m_node_info_storage->get_info(node_name);

        reply(std::move(response), 200);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Node %s not exists.", node_name.c_str());
        reply.error_json(404, e.what());
    }
}

void ros_support::handle_topics(
    [[maybe_unused]] clover2_http::http::core::request_context ctx,
    clover2_http::http::endpoint::deferred_reply<topics> reply) {
    topics response;
    const auto graph = m_node_context->get_node_graph_interface();

    for (const auto& [name, types] : graph->get_topic_names_and_types()) {
        response.topics.push_back(
            {name, types.empty() ? std::string{} : types.front()});
    }

    reply(std::move(response), 200);
}

void ros_support::handle_topic_json_stream(
    std::shared_ptr<clover2_http::http::transport::base_ws_session> session) {
    try {
        const auto& ctx = session->context();
        std::string topic = ctx.param<std::string>("topic");
        if (topic.empty() || topic.front() != '/') {
            topic.insert(topic.begin(), '/');
        }

        const auto graph = m_node_context->get_node_graph_interface();
        const auto topics_and_types = graph->get_topic_names_and_types();
        const auto found = topics_and_types.find(topic);
        if (found == topics_and_types.end() || found->second.empty()) {
            session->write_text(R"({"error":"unknown topic"})");
            session->close(1008);
            return;
        }

        const std::string type_name = found->second.front();

        auto state = std::make_shared<stream_state>(type_name, m_rate_limit);
        state->subscriber = std::make_unique<
            clover2_http::plugins::utils::universal_subscriber>(
            m_node_context, topic, type_name,
            [session,
             state](std::shared_ptr<rclcpp::SerializedMessage> serialized) {
                auto j = deserialize_message(serialized, state);

                if (j.has_value()) {
                    session->write_text(j->dump());
                }
            });

        session->on_close(
            [state](
                std::shared_ptr<clover2_http::http::transport::base_ws_session>,
                int) { state->subscriber.reset(); });

        session->start_reading();
    } catch (const std::exception& e) {
        session->write_text(nlohmann::json{{"error", e.what()}}.dump());
        session->close(1011);
    }
}

}  // namespace clover2_http::plugins

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_http::plugins::ros_support,
                       clover2_http::base_plugin)
