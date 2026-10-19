/**
 * @file base_interface.hpp
 * @brief Provides the base runtime interface passed to capability
 *        providers.
 */

#pragma once

// clover2
#include <clover2_common/node_context.hpp>

// third party
#include <yaml-cpp/yaml.h>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>
#include <string>
#include <utility>

namespace clover2_fcu::capability {

/**
 * @brief Runtime environment of capability providers.
 *
 * The first interface every provider depends on: node plumbing (clock,
 * logger, node interfaces) and the bridge node name — and nothing else.
 * Other capabilities are reached through the backend factory cache, not
 * through this interface.
 *
 * It follows the capability contract shape (name, shared_ptr) so the
 * factory serves it from the same cache as any other dependency —
 * builders pull it with get<base_interface>(), no special case.
 *
 * Passed by reference (ros2 node_interfaces style); the concrete
 * implementation is @ref base_interface_impl, created once by the
 * client.
 */
class base_interface {
public:
    using shared_ptr = std::shared_ptr<base_interface>;

    /// @brief Cache key in the backend factory (not a served capability).
    static constexpr const char* name = "base";

    virtual ~base_interface() = default;

    /** @brief The hosting node logger. */
    virtual rclcpp::Logger logger() const = 0;

    /** @brief The hosting node clock. */
    virtual rclcpp::Clock::SharedPtr clock() = 0;

    /** @brief The bridge node name (absolute), e.g. "/fcu_bridge". */
    virtual const std::string& bridge_node() const = 0;

    /**
     * @brief The bridge plugin_params (YAML map keyed by capability name).
     *
     * Static for the client lifetime; never null (an empty map when the
     * bridge configures no params). Providers read their own subtree via
     * the params(name) helper; missing sections yield an undefined node.
     */
    virtual const YAML::Node& params() const = 0;

    /** @brief The params subtree of one capability (undefined if absent). */
    YAML::Node params(const std::string& capability) const {
        return params()[capability];
    }

    /** @brief Node interfaces for creating subscriptions/timers/clients. */
    virtual clover2_common::node_context& node() = 0;
};

/**
 * @brief Default implementation of @ref base_interface.
 *
 * Owns a @ref clover2_common::node_context as a member, not a base: the
 * interface stays free of node plumbing details, and the implementation
 * is created once by the client and shared with the backend factory.
 */
class base_interface_impl : public base_interface {
public:
    /**
     * @brief Construct from a hosting node.
     * @tparam node_t Node type (clover2_common::node, lifecycle node, ...).
     * @param node the hosting node (must be spun while this runs).
     * @param bridge_node the bridge node name (absolute).
     */
    template <typename node_t>
    base_interface_impl(node_t& node, std::string bridge_node)
        : m_ctx(node), m_bridge_node(std::move(bridge_node)) {}

    rclcpp::Logger logger() const override {
        return m_ctx.get_logger();
    }

    rclcpp::Clock::SharedPtr clock() override {
        return m_ctx.get_node_clock_interface()->get_clock();
    }

    const std::string& bridge_node() const override {
        return m_bridge_node;
    }

    const YAML::Node& params() const override { return m_params; }

    /// @brief Called by the client once the bridge config arrives.
    void params(YAML::Node value) { m_params = std::move(value); }

    clover2_common::node_context& node() override {
        return m_ctx;
    }

private:
    clover2_common::node_context m_ctx;
    std::string m_bridge_node;
    YAML::Node m_params = YAML::Load("{}");
};

}  // namespace clover2_fcu::capability
