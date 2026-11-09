// clover2
#include "stub_backend.hpp"

// third party
#include <pluginlib/class_list_macros.hpp>

// STL
#include <utility>

namespace clover2_fcu::stub {

system_state::~system_state() {
    const std::scoped_lock lock(providers_mutex());
    std::erase_if(providers(), [this](const auto& weak) {
        return weak.expired() || weak.lock().get() == this;
    });
}

void system_state::publish(const data::system_state& state) {
    std::vector<std::shared_ptr<system_state>> live;
    {
        const std::scoped_lock lock(providers_mutex());
        for (const auto& weak : providers()) {
            if (const auto provider = weak.lock()) {
                live.push_back(provider);
            }
        }
        std::erase_if(providers(),
                      [](const auto& weak) { return weak.expired(); });
    }

    // Outside the lock: update_data fires subscriber callbacks.
    for (const auto& provider : live) {
        provider->inject(state);
    }
}

void system_state::register_provider(const std::shared_ptr<system_state>& p) {
    const std::scoped_lock lock(providers_mutex());
    providers().push_back(p);
}

std::mutex& system_state::providers_mutex() {
    static std::mutex mutex;
    return mutex;
}

std::vector<std::weak_ptr<system_state>>& system_state::providers() {
    static std::vector<std::weak_ptr<system_state>> providers;
    return providers;
}

void commands::arm(result_callback cb, bool run_preflight_checks) {
    (void)run_preflight_checks;
    issue("arm", std::move(cb));
}

void commands::disarm(result_callback cb) { issue("disarm", std::move(cb)); }

void commands::set_mode(result_callback cb, data::flight_mode mode) {
    issue("set_mode:" + std::string(mode.to_string()), std::move(cb));
}

void commands::takeoff(result_callback cb, std::optional<double> altitude,
                       std::optional<double> heading) {
    (void)altitude;
    (void)heading;
    issue("takeoff", std::move(cb));
}

void commands::land(result_callback cb) { issue("land", std::move(cb)); }

void commands::rtl(result_callback cb) { issue("rtl", std::move(cb)); }

std::size_t commands::pending_count(const std::string& name) const {
    const std::scoped_lock lock(m_mutex);
    const auto it = m_pending.find(name);
    return it == m_pending.end() ? 0 : it->second.size();
}

void commands::resolve_pending(const std::string& name, bool ok) {
    std::vector<result_callback> callbacks;
    {
        const std::scoped_lock lock(m_mutex);
        const auto it = m_pending.find(name);
        if (it == m_pending.end()) {
            return;
        }
        callbacks = std::move(it->second);
        m_pending.erase(it);
    }

    for (auto& cb : callbacks) {
        cb(ok);
    }
}

void commands::issue(std::string name, result_callback cb) {
    const std::scoped_lock lock(m_mutex);
    m_pending[std::move(name)].push_back(std::move(cb));
}

void position_setpoint::update(const Eigen::Vector3d& position,
                               std::optional<double> heading) {
    const std::scoped_lock lock(m_mutex);
    m_updates.push_back({position, heading});
}

std::vector<position_setpoint::update_record> position_setpoint::updates()
    const {
    const std::scoped_lock lock(m_mutex);
    return m_updates;
}

factory::factory() {
    add<capability::system_state_interface>([](const context& ctx) {
        auto impl = std::make_shared<system_state>(ctx);
        system_state::register_provider(impl);
        return impl;
    });
    add<capability::commands_interface>(
        [](const context& ctx) { return std::make_shared<commands>(ctx); });
    add<capability::setpoint_position_interface>([](const context& ctx) {
        return std::make_shared<position_setpoint>(ctx);
    });
}

std::string factory::protocol() const { return "stub"; }

void factory::do_initialize() {}

}  // namespace clover2_fcu::stub

PLUGINLIB_EXPORT_CLASS(clover2_fcu::stub::factory,
                       clover2_fcu::backend::backend_base)
