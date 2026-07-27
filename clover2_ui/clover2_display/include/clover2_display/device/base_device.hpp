#pragma once

#include <clover2_common/node_context.hpp>
#include <clover2_display/data/display_frame.hpp>
#include <clover2_display/data/display_info.hpp>
#include <rclcpp/rclcpp.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace clover2_display::device {

class base_device {
public:
    virtual ~base_device() = default;

    void initialize(const std::string& name,
                    std::shared_ptr<clover2_common::node_context> node_context);
    void cleanup() noexcept;

    const clover2_display::data::display_info& info() const noexcept;

    void write(const clover2_display::data::display_frame& frame);

    const std::string& get_name() const;

protected:
    explicit base_device();

    clover2_display::data::display_info& info() noexcept;

    virtual void on_initialize() = 0;
    virtual void on_cleanup() = 0;
    virtual void write_raw_frame(
        const clover2_display::data::display_frame& frame) = 0;

    rclcpp::Logger get_logger() const;
    rclcpp::Clock::SharedPtr get_clock() const;
    std::shared_ptr<clover2_common::node_context> get_node_context() const;

private:
    clover2_display::data::display_info m_info;

    std::string m_name;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2_display::device
