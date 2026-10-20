#pragma once

// clover2
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu/capability/all.hpp>
#include <clover2_fcu/capability/setpoint_base.hpp>
#include <clover2_fcu/common/exception.hpp>

// third party
#include <yaml-cpp/yaml.h>

// STL
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace clover2_fcu::backend {

/**
 * @brief Base class of a backend (pluginlib base).
 *
 * One pluginlib class per backend package; the lookup name must equal
 * protocol() (validated by the bridge).
 *
 * The backend is the composition root of one client. Builders registered
 * with add() create capability providers; get() caches each provider for
 * the client lifetime. Provider dependencies are declared through their
 * constructors and resolved inside builder lambdas via get() — never
 * inside providers themselves:
 *
 *   add<estimator_interface>([this] {
 *       return std::make_shared<estimator_provider>(
 *           get<capability::base_interface>(), get<parameters_interface>());
 *   });
 *
 * Configuration is not a dependency: providers read their params subtree
 * via capability::base_interface::params(name) (the bridge plugin_params,
 * a YAML map keyed by capability name, published by the client).
 *
 * Setpoint providers are the exception: they are bound to a flight mode
 * at construction (mode_token) and are never cached — create_setpoint()
 * makes a new provider per call and is reachable only from mode_base.
 *
 * Lifecycle: the client calls initialize() once before the first
 * get()/create_setpoint() and seeds the runtime environment
 * (base_interface, with the params inside) into the cache; the bridge
 * reads only protocol()/capabilities() and never initializes the backend.
 *
 * One factory instance per client (each client loads its own), so the
 * cache is per client. Thread-safe: the lazy provider cache and the
 * loaded flag are guarded by a recursive mutex (builders resolve their
 * dependencies via get() re-entrantly, under the same lock); the
 * builder maps are immutable after construction and read lock-free.
 */
class backend_base {
public:
    using shared_ptr = std::shared_ptr<backend_base>;
    using unique_ptr = std::unique_ptr<backend_base>;

    /// @brief Creates one cached capability provider.
    template <typename CapabilityT>
    using builder_type = std::function<std::shared_ptr<CapabilityT>()>;

    /// @brief Creates one setpoint provider bound to the mode.
    template <typename SetpointT>
    using setpoint_builder_type =
        std::function<std::shared_ptr<SetpointT>(mode_token)>;

    virtual ~backend_base() = default;

    /// @brief Backend (protocol) name used as the default plugin.
    virtual std::string protocol() const = 0;

    /// @brief Mode registration hook, called by the bridge from its
    ///        register_mode service.
    ///
    /// Backends whose FCU supports native external-mode registration
    /// duplicate it here (e.g. px4 RegisterExtComponent); the default
    /// does nothing — the bridge lease is the only arbiter.
    /// @return backend-native mode id, 0 when the backend has none.
    virtual uint32_t register_mode(const std::string& name) {
        (void)name;
        return 0;
    }

    /// @brief Mode unregistration hook, called by the bridge when a mode
    ///        is removed (bond broken / cleanup). Default does nothing.
    virtual void unregister_mode(const std::string& name) { (void)name; }

    /// @brief Names of the capabilities served by this backend (cached
    ///        capabilities and setpoints).
    std::vector<std::string> capabilities() const;

    /**
     * @brief Initialize the backend and seed the runtime environment.
     *
     * Called by the client once before the first get()/create_setpoint().
     * @param base the client runtime environment (params included), cached
     *        under base_interface::name for the builders.
     */
    void initialize(std::shared_ptr<capability::base_interface> base);

    /**
     * @brief The cached provider for the capability contract.
     *
     * The first call creates the provider via the registered builder, the
     * same instance is returned afterwards. The runtime environment is a
     * cached dependency too: get<base_interface>() (seeded by
     * initialize()).
     * @throws clover2_fcu::exception when the backend is not initialized
     *         or the capability is not registered.
     */
    template <capability::contract CapabilityT>
    std::shared_ptr<CapabilityT> get() const {
        return std::static_pointer_cast<CapabilityT>(get(CapabilityT::name));
    }

    /// @brief The cached provider for the capability name.
    /// @throws clover2_fcu::exception when the backend is not initialized
    ///         or the capability is not registered.
    std::shared_ptr<void> get(const std::string& capability) const;

    /**
     * @brief Create a new setpoint provider bound to the mode.
     *
     * Setpoints are never cached: one provider per call, owned by the
     * caller (mode_base).
     * @throws clover2_fcu::exception when the backend is not initialized
     *         or the setpoint is not registered.
     */
    template <capability::setpoint_contract SetpointT>
    std::shared_ptr<SetpointT> create_setpoint(mode_token token) const {
        return std::static_pointer_cast<SetpointT>(
            create_setpoint(SetpointT::name, token));
    }

protected:
    /**
     * @brief Register the builder of a cached capability provider.
     *
     * The builder resolves its dependencies via get() (base_interface,
     * other capabilities); providers reach their config via
     * base_interface::params(name).
     */
    template <capability::contract CapabilityT>
    void add(builder_type<CapabilityT> builder) {
        m_builders.emplace(
            CapabilityT::name,
            [builder = std::move(builder)]() -> std::shared_ptr<void> {
                return builder();
            });
    }

    /**
     * @brief Register the builder of a setpoint provider (never cached).
     *
     * The builder receives the mode token and resolves its dependencies
     * via get().
     */
    template <capability::setpoint_contract SetpointT>
    void add_setpoint(setpoint_builder_type<SetpointT> builder) {
        m_setpoint_builders.emplace(
            SetpointT::name,
            [builder = std::move(builder)](mode_token token)
                -> std::shared_ptr<void> { return builder(token); });
    }

    /**
     * @brief Backend initialization hook (transport setup etc.).
     *
     * Called once from initialize(); the environment is already seeded.
     */
    virtual void do_initialize(
        std::shared_ptr<capability::base_interface> base) = 0;

private:
    /// @brief Create a new setpoint provider by name.
    /// @throws clover2_fcu::exception when not initialized or the
    ///         setpoint is not registered.
    std::shared_ptr<void> create_setpoint(const std::string& setpoint,
                                          mode_token token) const;

    std::unordered_map<std::string,
                       std::function<std::shared_ptr<void>()>>
        m_builders;
    std::unordered_map<
        std::string,
        std::function<std::shared_ptr<void>(mode_token)>>
        m_setpoint_builders;
    mutable std::unordered_map<std::string, std::shared_ptr<void>> m_cache;

    /// @brief Guards m_cache, m_loaded and running builders (recursive:
    ///        builders call get() for their dependencies).
    mutable std::recursive_mutex m_mutex;

    bool m_loaded{false};
};

}  // namespace clover2_fcu::backend
