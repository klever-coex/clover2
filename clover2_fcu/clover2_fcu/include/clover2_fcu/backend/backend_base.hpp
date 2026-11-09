#pragma once

// clover2
#include <clover2_fcu/capability/all.hpp>
#include <clover2_fcu/common/exception.hpp>
#include <clover2_fcu/context.hpp>

// third party
#include <yaml-cpp/yaml.h>

// STL
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace clover2_fcu::backend {

class backend_base {
public:
    using shared_ptr = std::shared_ptr<backend_base>;
    using unique_ptr = std::unique_ptr<backend_base>;

    template <typename T>
    using builder_type = std::function<std::shared_ptr<T>(const context&)>;
    using builder = std::function<std::shared_ptr<void>(const context&)>;

    virtual ~backend_base() = default;

    /// @brief Protocol id; must equal the pluginlib lookup name
    virtual std::string protocol() const = 0;

    /// @brief FCU-native mode registration; backends without native
    ///        support return nullopt (the bridge keeps its own registry).
    virtual std::optional<uint32_t> register_mode(const std::string& name) {
        (void)name;
        return std::nullopt;
    }

    virtual void unregister_mode(const std::string& name) { (void)name; }

    /// @brief Names of the capabilities this factory serves.
    std::vector<std::string> capabilities() const;

    /// @brief Store the client-assembled environment and run do_initialize().
    void initialize(const context& ctx);

    /// @brief Build a provider via its builder (not cached here — the
    ///        client caches).
    std::shared_ptr<void> get(const std::string& capability) const;

protected:
    template <capability::contract CapabilityT>
    void add(builder_type<CapabilityT> builder) {
        m_builders.emplace(CapabilityT::name,
                           [builder = std::move(builder)](
                               const context& ctx) -> std::shared_ptr<void> {
                               return builder(ctx);
                           });
    }

    virtual void do_initialize() = 0;

    /// @brief The environment stored at initialize() (factory setup).
    const context& fcu_context() const { return m_context; }

private:
    std::unordered_map<std::string, builder> m_builders;
    context m_context;

    bool m_loaded{false};
};

}  // namespace clover2_fcu::backend
