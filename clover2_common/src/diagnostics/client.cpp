#include <clover2_common/diagnostics/client.hpp>

#include <stdexcept>
#include <utility>

namespace clover2_common::diagnostics {

void client::initialize(std::shared_ptr<node_context> node_context,
                        const std::string& topic, callback_type callback) {
    if (!node_context) {
        throw std::invalid_argument("Diagnostics client received null context");
    }
    if (!callback) {
        throw std::invalid_argument(
            "Diagnostics client received empty callback");
    }

    m_node_context = std::move(node_context);
    m_callback = std::move(callback);

    m_sub = rclcpp::create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
        m_node_context, topic, rclcpp::QoS(10),
        std::bind(&client::diagnostics_callback, this, std::placeholders::_1));
}

void client::cleanup() {
    m_sub.reset();
    m_callback = nullptr;
    m_node_context.reset();
}

void client::diagnostics_callback(
    diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg) {
    for (const auto& status : msg->status) {
        m_callback(status);
    }
}

bool client::status_changed(const status_type& previous,
                            const status_type& current) {
    return previous.level != current.level ||
           previous.name != current.name ||
           previous.message != current.message;
}

}  // namespace clover2_common::diagnostics
