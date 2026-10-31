#pragma once

// clover2
#include <clover2_fcu/capability/all.hpp>
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu/capability/setpoint_base.hpp>
#include <clover2_fcu/common/exception.hpp>

// third party
#include <yaml-cpp/yaml.h>

// STL
#include <functional>
#include <mutex>
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
    using builder_type = std::function<std::shared_ptr<T>()>;
    using builder = std::function<std::shared_ptr<void>()>;

    template <typename T>
    using setpoint_builder_type = std::function<std::shared_ptr<T>(mode_token)>;
    using setpoint_builder = std::function<std::shared_ptr<void>(mode_token)>;

    virtual ~backend_base() = default;

    virtual std::string protocol() const = 0;

    virtual std::optional<uint32_t> register_mode(const std::string& name) {
        (void)name;
        return std::nullopt;
    }

    virtual void unregister_mode(const std::string& name) { (void)name; }

    std::vector<std::string> capabilities() const;

    void initialize();

    std::shared_ptr<void> get(const std::string& capability) const;

    template <capability::setpoint_contract SetpointT>
    std::shared_ptr<SetpointT> create_setpoint(mode_token token) const {
        return std::static_pointer_cast<SetpointT>(
            create_setpoint(SetpointT::name, token));
    }

protected:
    template <capability::contract CapabilityT>
    void add(builder_type<CapabilityT> builder) {
        m_builders.emplace(
            CapabilityT::name,
            [builder = std::move(builder)]() -> std::shared_ptr<void> {
                return builder();
            });
    }

    template <capability::setpoint_contract SetpointT>
    void add_setpoint(setpoint_builder_type<SetpointT> builder) {
        m_setpoint_builders.emplace(
            SetpointT::name,
            [builder = std::move(builder)](mode_token token)
                -> std::shared_ptr<void> { return builder(token); });
    }

    virtual void do_initialize() = 0;

private:
    std::shared_ptr<void> create_setpoint(const std::string& setpoint,
                                          mode_token token) const;

    std::unordered_map<std::string, builder> m_builders;
    std::unordered_map<std::string, setpoint_builder> m_setpoint_builders;
    // mutable std::unordered_map<std::string, std::shared_ptr<void>> m_cache;

    bool m_loaded{false};
};

}  // namespace clover2_fcu::backend
