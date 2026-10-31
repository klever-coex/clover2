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

class base_interface {
public:
    using shared_ptr = std::shared_ptr<base_interface>;

    static constexpr const char* name = "base";

    virtual ~base_interface() = default;

    virtual rclcpp::Logger logger() const = 0;
    virtual rclcpp::Clock::SharedPtr clock() = 0;
    virtual const std::string& bridge_node() const = 0;

    virtual const YAML::Node& params() const = 0;
    YAML::Node params(const std::string& capability) const {
        return params()[capability];
    }

    virtual clover2_common::node_context& node() = 0;

    virtual rclcpp::CallbackGroup::SharedPtr callback_group() = 0;
};

class base_interface_impl : public base_interface {
public:
    template <typename node_t>
    base_interface_impl(node_t& node, std::string bridge_node,
                        rclcpp::CallbackGroup::SharedPtr group)
        : m_ctx(node)
        , m_bridge_node(std::move(bridge_node))
        , m_group(std::move(group)) {}

    rclcpp::Logger logger() const override { return m_ctx.get_logger(); }

    rclcpp::Clock::SharedPtr clock() override {
        return m_ctx.get_node_clock_interface()->get_clock();
    }

    const std::string& bridge_node() const override { return m_bridge_node; }

    const YAML::Node& params() const override { return m_params; }
    void params(YAML::Node value) { m_params = std::move(value); }

    clover2_common::node_context& node() override { return m_ctx; }

    rclcpp::CallbackGroup::SharedPtr callback_group() override {
        return m_group;
    }

private:
    clover2_common::node_context m_ctx;
    std::string m_bridge_node;
    rclcpp::CallbackGroup::SharedPtr m_group;
    YAML::Node m_params = YAML::Load("{}");
};

}  // namespace clover2_fcu::capability
