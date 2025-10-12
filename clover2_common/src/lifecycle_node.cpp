#include <clover2_common/lifecycle_node.hpp>
#include <lifecycle_msgs/msg/state.hpp>

#include <memory>
#include <string>
#include <vector>

namespace clover2_common {

lifecycle_node::lifecycle_node(const std::string &node_name,
                               const rclcpp::NodeOptions &options)
    : rclcpp_lifecycle::LifecycleNode(node_name, options) {
    declare_parameter("autostart", true);

    if (get_parameter("autostart").as_bool()) {
        m_init_timer =
            this->create_wall_timer(std::chrono::seconds(0), [this]() {
                configure();

                if (get_current_state().id() ==
                    lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE) {
                    activate();
                }

                m_init_timer.reset();
            });
    }
}

lifecycle_node::~lifecycle_node() { RCLCPP_INFO(get_logger(), "Destroying"); }

lifecycle_node::SetParametersResult lifecycle_node::on_set_parameters_cb(
    const std::vector<rclcpp::Parameter> &parameters) {
    SetParametersResult result;
    result.successful = true;

    for (auto &p : parameters) {
        auto it = m_watch_parameters.find(p.get_name());
        if (it != m_watch_parameters.end()) {
            try {
                it->second(p);
            } catch (std::exception &ex) {
                result.successful = false;
                result.reason = ex.what();
                RCLCPP_ERROR(get_logger(), "Fail set parameter `%s` with: %s",
                             p.get_name().c_str(), ex.what());
                break;
            }
        }
    }

    return result;
}

void lifecycle_node::enable_watch_parameters() {
    m_set_parameters_handle_ptr = add_on_set_parameters_callback(std::bind(
        &lifecycle_node::on_set_parameters_cb, this, std::placeholders::_1));
}

}  // namespace clover2_common
