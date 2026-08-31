#pragma once

#include <clover2_common/node_context.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <rclcpp/rclcpp.hpp>

#include <functional>
#include <memory>
#include <string>

namespace clover2_common::diagnostics {

class client {
public:
    using message_type = diagnostic_msgs::msg::DiagnosticArray;
    using status_type = diagnostic_msgs::msg::DiagnosticStatus;
    using callback_type = std::function<void(const message_type&)>;

    static constexpr const char* default_topic = "/diagnostics_agg";

    explicit client(std::shared_ptr<node_context> node_context,
                    const std::string& topic = default_topic);
    ~client() = default;

    void set_callback(callback_type callback);
    void cleanup();
    static bool status_changed(const status_type& previous,
                               const status_type& current);

private:
    void diagnostics_callback(message_type::SharedPtr msg);

    std::shared_ptr<node_context> m_node_context;
    callback_type m_callback;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        m_sub;
};

}  // namespace clover2_common::diagnostics
