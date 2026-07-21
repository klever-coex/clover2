#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_led/data/driver_info.hpp>
#include <clover2_led/data/led_frame.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <chrono>
#include <memory>

namespace clover2_led::device {

class base_device {
public:
    virtual ~base_device() = default;

    void initialize(const std::string& name, size_t led_count,
                    std::shared_ptr<clover2_common::node_context> node_context);
    void cleanup() noexcept;

    const clover2_led::data::driver_info& info() const noexcept;

    void write(const clover2_led::data::led_frame& frame);

    bool set_brightness(float brightness);
    float brightness() const;

    const std::string& get_name() const;

protected:
    explicit base_device();

    clover2_led::data::driver_info& info() noexcept;

    virtual void on_initialize(size_t led_count) = 0;
    virtual void on_cleanup() = 0;

    virtual bool set_hardware_brightness(float brightness);

    virtual void write_raw_frame(
        const std::vector<clover2_led::data::color>& colors) = 0;

    rclcpp::Logger get_logger() const;
    rclcpp::Clock::SharedPtr get_clock() const;
    std::shared_ptr<clover2_common::node_context> get_node_context() const;

private:
    clover2_led::data::driver_info m_info;

    float m_brightness{1.f};
    std::chrono::steady_clock::time_point m_last_write;
    std::vector<clover2_led::data::color> m_frame_buffer{};

    std::string m_name;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2_led::device
