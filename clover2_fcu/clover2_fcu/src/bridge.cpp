// clover2
#include <clover2_fcu/bridge.hpp>
#include <clover2_fcu/common/exception.hpp>

// STL
#include <algorithm>
#include <functional>
#include <utility>

namespace clover2_fcu {

bridge::bridge(const rclcpp::NodeOptions& options)
    : clover2_common::node("fcu_bridge", options) {
    declare_parameter<std::string>("protocol", "mavros");
    declare_parameter<std::string>("plugin_params", "");

    m_config.protocol = get_parameter("protocol").as_string();
    try {
        m_factory = m_loader.createSharedInstance(m_config.protocol);
    } catch (const pluginlib::PluginlibException& e) {
        throw exception(
            "failed to load the factory '{}' (is the backend package in the "
            "workspace?): {}",
            m_config.protocol, e.what());
    }

    m_config.plugin_params = get_parameter("plugin_params").as_string();

    m_get_config_service = create_service<clover2_fcu_msgs::srv::GetConfig>(
        "get_config", std::bind(&bridge::handle_get_config, this,
                                std::placeholders::_1, std::placeholders::_2));

    m_mode_state_pub = create_publisher<clover2_fcu_msgs::msg::ModeState>(
        "mode_state", rclcpp::QoS(1).transient_local());
    publish_mode_state();

    m_register_mode_service =
        create_service<clover2_fcu_msgs::srv::RegisterMode>(
            "register_mode",
            std::bind(&bridge::handle_register_mode, this,
                      std::placeholders::_1, std::placeholders::_2));
    m_activate_mode_service =
        create_service<clover2_fcu_msgs::srv::ActivateMode>(
            "activate_mode",
            std::bind(&bridge::handle_activate_mode, this,
                      std::placeholders::_1, std::placeholders::_2));
}

bridge::~bridge() {
    m_factory.reset();
}

void bridge::handle_get_config(
    const std::shared_ptr<clover2_fcu_msgs::srv::GetConfig::Request>,
    std::shared_ptr<clover2_fcu_msgs::srv::GetConfig::Response> response) {
    response->ok = true;
    response->config = m_config;
}

void bridge::handle_register_mode(
    const std::shared_ptr<clover2_fcu_msgs::srv::RegisterMode::Request> request,
    std::shared_ptr<clover2_fcu_msgs::srv::RegisterMode::Response> response) {
    std::erase_if(m_modes, [this](const auto& entry) {
        if (entry.second.broken) {
            m_factory->unregister_mode(entry.second.name);
            return true;
        }

        return false;
    });

    if (request->name.empty()) {
        response->ok = false;
        response->error = "mode name must not be empty";
        return;
    }

    for (const auto& [mode_id, record] : m_modes) {
        if (record.name == request->name) {
            response->ok = false;
            response->error =
                "mode '" + request->name + "' is already registered";
            return;
        }
    }

    const uint32_t native_mode_id = m_factory->register_mode(request->name);

    const uint32_t mode_id = m_next_mode_id++;
    auto mode_bond = std::make_shared<bond::Bond>(
        "bond", std::to_string(mode_id), shared_from_this(),
        [this, mode_id]() { handle_mode_bond_broken(mode_id); });
    mode_bond->setHeartbeatPeriod(0.10);
    mode_bond->start();

    m_modes.emplace(mode_id, mode_record{request->name, std::move(mode_bond),
                                         false, native_mode_id});

    response->ok = true;
    response->mode_id = mode_id;
    RCLCPP_INFO(get_logger(), "mode '%s' registered (id=%u, native=%u)",
                request->name.c_str(), mode_id, native_mode_id);
}

void bridge::handle_activate_mode(
    const std::shared_ptr<clover2_fcu_msgs::srv::ActivateMode::Request> request,
    std::shared_ptr<clover2_fcu_msgs::srv::ActivateMode::Response> response) {
    uint32_t target_mode_id = 0;
    if (!request->name.empty()) {
        const auto it = std::find_if(
            m_modes.begin(), m_modes.end(), [&request](const auto& entry) {
                return entry.second.name == request->name;
            });

        if (it == m_modes.end() || it->second.broken) {
            response->ok = false;
            response->error = "mode '" + request->name + "' is not registered";
            return;
        }

        target_mode_id = it->first;
    }

    if (target_mode_id != m_active_mode_id) {
        m_active_mode_id = target_mode_id;
        publish_mode_state();
    }

    response->ok = true;
}

void bridge::handle_mode_bond_broken(uint32_t mode_id) {
    const auto it = m_modes.find(mode_id);
    if (it == m_modes.end() || it->second.broken) {
        return;
    }

    RCLCPP_WARN(get_logger(), "mode '%s' (id=%u) bond broken, unregistering",
                it->second.name.c_str(), mode_id);

    it->second.broken = true;

    if (m_active_mode_id == mode_id) {
        m_active_mode_id = 0;
        publish_mode_state();
    }
}

void bridge::publish_mode_state() {
    clover2_fcu_msgs::msg::ModeState state;
    state.mode_id = m_active_mode_id;
    if (m_active_mode_id != 0) {
        const auto it = m_modes.find(m_active_mode_id);
        state.name = (it != m_modes.end()) ? it->second.name : "";
    }

    m_mode_state_pub->publish(state);
}

}  // namespace clover2_fcu

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_fcu::bridge)
