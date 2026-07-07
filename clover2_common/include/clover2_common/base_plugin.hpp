#pragma once

#include <clover2_common/node_context.hpp>

namespace clover2_common {

class base_plugin {
public:
    virtual ~base_plugin() = default;

protected:
    explicit base_plugin();

    void initialize(const std::string& name,
                    std::shared_ptr<clover2_common::node_context> node_context);
    void cleanup() noexcept;

    const std::string& get_name() const;

    rclcpp::Logger get_logger() const;
    rclcpp::Clock::SharedPtr get_clock() const;

    virtual void on_initialize();
    virtual void on_cleanup() noexcept;

    std::shared_ptr<clover2_common::node_context> m_node_context;

private:
    std::string m_name;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
};

}  // namespace clover2_common
