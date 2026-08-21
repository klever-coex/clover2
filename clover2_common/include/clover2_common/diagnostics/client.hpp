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
    using status_type = diagnostic_msgs::msg::DiagnosticStatus;
    using callback_type = std::function<void(const status_type&)>;

    client() = default;
    ~client() = default;

    void initialize(std::shared_ptr<node_context> node_context,
                    const std::string& topic, callback_type callback);
    void cleanup();
    static bool status_changed(const status_type& previous,
                               const status_type& current);

private:
    void diagnostics_callback(
        diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    std::shared_ptr<node_context> m_node_context;
    callback_type m_callback;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        m_sub;
};

}  // namespace clover2_common::diagnostics