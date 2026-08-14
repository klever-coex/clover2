#include <clover2_http/plugin.hpp>
#include <clover2_http_plugins/data/topic_ref.hpp>
#include <clover2_http_plugins/utils/message_type.hpp>
#include <clover2_http_plugins/utils/msg_json.hpp>
#include <clover2_http_plugins/utils/rate_limiter.hpp>
#include <clover2_http_plugins/utils/universal_subscriber.hpp>

#include <pluginlib/class_list_macros.hpp>
#include <rclcpp/serialized_message.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace clover2_http_plugins {

struct topics_response {
    std::vector<data::topic_ref> topics;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(topics_response, topics)

class topics_plugin : public clover2_http::plugin<topics_plugin> {
public:
    static constexpr std::string_view k_name = "topics";
    static constexpr int k_version = 1;

protected:
    void on_initialize() override {
        m_rate_limit = m_node_context->get_node_parameters_interface()
                           ->declare_parameter("topics.rate_limit_bps",
                                               rclcpp::ParameterValue(100000.0))
                           .get<double>();

        m_server->get<void, topics_response>(
            "/topics",
            [node_context = m_node_context](
                clover2_http::http::core::request_context,
                clover2_http::http::endpoint::reply<topics_response> reply) {
                topics_response response;
                const auto graph = node_context->get_node_graph_interface();

                for (const auto& [name, types] :
                     graph->get_topic_names_and_types()) {
                    response.topics.push_back(
                        {name, types.empty() ? std::string{} : types.front()});
                }

                reply(response, 200);
            });

        m_server->raw_ws("/topic/-/{topic...}",
                         std::bind(&topics_plugin::handle_topic_stream, this,
                                   std::placeholders::_1));
    }

    std::vector<std::string> capabilities() const override {
        return {"topics"};
    }

private:
    void handle_topic_stream(
        std::shared_ptr<clover2_http::http::transport::base_ws_session>
            session) {
        try {
            const auto& ctx = session->context();
            std::string topic = ctx.path_params.at("topic");
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

            auto state =
                std::make_shared<stream_state>(type_name, m_rate_limit);
            state->subscriber = std::make_unique<utils::universal_subscriber>(
                m_node_context, topic, type_name,
                [session,
                 state](std::shared_ptr<rclcpp::SerializedMessage> serialized) {
                    try {
                        if (!state->limiter.allow(serialized->size())) {
                            return;
                        }

                        void* msg = state->type.allocate();
                        if (!state->type.deserialize(*serialized, msg)) {
                            state->type.deallocate(msg);
                            return;
                        }

                        const auto json =
                            clover2_http_plugins::utils::msg_json::detail::
                                to_json(state->type.members(), msg);

                        state->type.deallocate(msg);
                        session->write_text(json.dump());
                    } catch (const std::exception& e) {
                        session->write_text(
                            nlohmann::json{{"error", e.what()}}.dump());
                    }
                });

            session->on_close(
                [state](std::shared_ptr<
                            clover2_http::http::transport::base_ws_session>,
                        int) { state->subscriber.reset(); });

            session->start_reading();
        } catch (const std::exception& e) {
            session->write_text(nlohmann::json{{"error", e.what()}}.dump());
            session->close(1011);
        }
    }

    struct stream_state {
        stream_state(const std::string& type_name, double bytes_per_second)
            : type(type_name)
            , limiter(bytes_per_second) {}

        utils::message_type type;
        utils::rate_limiter limiter;
        std::unique_ptr<utils::universal_subscriber> subscriber;
    };

    double m_rate_limit = 10.0;
};

}  // namespace clover2_http_plugins

PLUGINLIB_EXPORT_CLASS(clover2_http_plugins::topics_plugin,
                       clover2_http::base_plugin)
