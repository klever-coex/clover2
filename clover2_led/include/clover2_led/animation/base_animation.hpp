#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_led/data/driver_info.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>

namespace clover2_led::animation {

class base_animation {
public:
    virtual ~base_animation() = default;

protected:
    explicit base_animation();

    void initialize(const std::string& name,
                    std::shared_ptr<clover2_common::node_context> node_context);
    void cleanup() noexcept;

    const std::string& get_name() const;

    rclcpp::Logger get_logger() const;
    rclcpp::Clock::SharedPtr get_clock() const;

private:

    std::vector<clover2_led::data::color> m_frame_buffer{};

    std::string m_name;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2_led::animation
