#pragma once

// clover2
#include <clover2_fcu/capability/base_interface.hpp>

// STL
#include <memory>

namespace clover2_fcu_mavros::capability {

class base : public clover2_fcu::capability::base_interface {
public:
    using shared_ptr = std::shared_ptr<base>;

    template <typename node_t>
    base(node_t& node, std::string bridge_node)
        : m_ctx(node)
        , m_bridge_node(std::move(bridge_node)) {}

    virtual ~base() = default;

    rclcpp::Logger logger() const override { return m_ctx.get_logger(); }

    rclcpp::Clock::SharedPtr clock() override {
        return m_ctx.get_node_clock_interface()->get_clock();
    }

    const std::string& bridge_node() const override { return m_bridge_node; }

    const YAML::Node& params() const override { return m_params; }

    /// @brief Called by the client once the bridge config arrives.
    void params(YAML::Node value) { m_params = std::move(value); }

    clover2_common::node_context& node() override { return m_ctx; }

private:
    clover2_common::node_context m_ctx;
    std::string m_bridge_node;
    YAML::Node m_params = YAML::Load("{}");
};

}  // namespace clover2_fcu_mavros::capability
