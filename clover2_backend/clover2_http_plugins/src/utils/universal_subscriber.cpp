#include <clover2_http_plugins/utils/universal_subscriber.hpp>

#include <rclcpp/create_generic_subscription.hpp>
#include <rclcpp/generic_subscription.hpp>

namespace clover2_http_plugins::utils {

universal_subscriber::universal_subscriber(
    std::shared_ptr<clover2_common::node_context> node_context,
    const std::string& topic_name, const std::string& topic_type,
    callback cb) {
    m_subscription = rclcpp::create_generic_subscription(
        node_context->get_node_topics_interface(), topic_name, topic_type,
        rclcpp::QoS(10), std::move(cb));
}

universal_subscriber::~universal_subscriber() = default;

}  // namespace clover2_http_plugins::utils
