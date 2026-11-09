// clover2
#include <clover2_common/util/sync_service.hpp>
#include <clover2_fcu/client.hpp>

// STL
#include <algorithm>
#include <utility>

namespace clover2_fcu {

using clover2_common::util::call_sync;

client::client(client::required_interfaces node_interfaces, client::settings s)
    : m_settings(std::move(s)) {
    m_context.logger =
        node_interfaces.get_node_logging_interface()->get_logger();
    m_context.clock = node_interfaces.get_node_clock_interface()->get_clock();
    m_context.node = node_interfaces;
    m_context.group =
        node_interfaces.get_node_base_interface()->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive);
    m_context.bridge_node = m_settings.bridge_node();

    m_config_client = m_context.client<clover2_fcu_msgs::srv::GetConfig>(
        m_settings.bridge_node() + "/get_config");

    connect();
}

void client::connect() {
    const std::string service_name = m_settings.bridge_node() + "/get_config";

    if (!m_config_client->wait_for_service(m_settings.wait_timeout())) {
        throw exception("fcu bridge is not available: {}", service_name);
    }

    std::shared_ptr<clover2_fcu_msgs::srv::GetConfig::Response> response;
    try {
        response = call_sync<clover2_fcu_msgs::srv::GetConfig>(
            m_config_client, service_name,
            std::make_shared<clover2_fcu_msgs::srv::GetConfig::Request>(),
            m_settings.wait_timeout());
    } catch (const std::runtime_error& e) {
        throw exception("fcu bridge error: {}", e.what());
    }

    if (!response->ok) {
        throw exception("fcu bridge rejected the configuration request: {}",
                        response->error);
    }

    apply_config(response->config);
}

std::shared_ptr<void> client::get(const std::string& capability) {
    // Held across the builder call: two threads racing on a cold capability
    // must not create two providers (the cache is the client's contract).
    const std::lock_guard<std::mutex> lock(m_cache_mutex);

    const auto cached = m_cache.find(capability);
    if (cached != m_cache.end()) {
        return cached->second;
    }

    auto provider = backend()->get(capability);
    m_cache.emplace(capability, provider);

    return provider;
}

void client::apply_config(const clover2_fcu_msgs::msg::FcuConfig& config) {
    if (!config.plugin_params.empty()) {
        try {
            m_context.plugin_params = YAML::Load(config.plugin_params);
        } catch (const YAML::Exception& e) {
            throw exception("invalid plugin params yaml: {}", e.what());
        }

        if (!m_context.plugin_params.IsNull() &&
            !m_context.plugin_params.IsMap()) {
            throw exception(
                "plugin params yaml must be a map keyed by capability name");
        }
    }

    m_config = config;

    check_requirements();
}

void client::check_requirements() {
    std::string missing;
    for (const std::string& capability_name : m_settings.required()) {
        if (!has(capability_name)) {
            if (!missing.empty()) {
                missing += ", ";
            }

            missing += capability_name;
        }
    }

    if (!missing.empty()) {
        throw exception(
            "required capabilities not served by the resolved factories: {}",
            missing);
    }
}

bool client::has(const std::string& capability) {
    try {
        const auto capabilities = backend()->capabilities();
        return std::find(capabilities.begin(), capabilities.end(),
                         capability) != capabilities.end();
    } catch (const exception&) {
        return false;
    }
}

backend::backend_base::shared_ptr client::backend() {
    const std::lock_guard<std::mutex> lock(m_backend_mutex);
    auto backend_instance = ensure_backend(m_config.protocol);
    backend_instance->initialize(m_context);

    return backend_instance;
}

backend::backend_base::shared_ptr client::ensure_backend(
    const std::string& plugin_name) {
    if (m_backend) {
        return m_backend;
    }

    try {
        m_backend = m_loader.createSharedInstance(plugin_name);
    } catch (const pluginlib::PluginlibException& e) {
        throw exception(
            "failed to load the factory '{}' (is the backend package in the "
            "workspace?): {}",
            plugin_name, e.what());
    }

    if (m_backend->protocol() != plugin_name) {
        throw exception(
            "factory lookup name '{}' does not match its protocol() '{}' "
            "(check the backend plugins.xml)",
            plugin_name, m_backend->protocol());
    }

    return m_backend;
}

}  // namespace clover2_fcu
