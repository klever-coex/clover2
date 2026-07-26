#include <clover2_display/device/base_device.hpp>

namespace clover2_display::device {

base_device::base_device()
    : m_logger(rclcpp::get_logger("base_device"))
    , m_clock(nullptr)
    , m_node_context(nullptr) {}

void base_device::initialize(
    const std::string& name,
    std::shared_ptr<clover2_common::node_context> node_context) {
    m_node_context = std::move(node_context);

    m_name = name;
    m_logger = m_node_context->get_logger().get_child(m_name);
    m_clock = m_node_context->get_node_clock_interface()->get_clock();

    on_initialize();
    m_last_write = std::chrono::steady_clock::now();
}

void base_device::cleanup() noexcept {
    on_cleanup();
    m_node_context.reset();
}

const data::display_info& base_device::info() const noexcept { return m_info; }
data::display_info& base_device::info() noexcept { return m_info; }

void base_device::write(const data::display_frame& frame) {
    // TODO: Validate params
    write_raw_frame(frame);
    m_last_write = std::chrono::steady_clock::now();
}

const std::string& base_device::get_name() const { return m_name; }

rclcpp::Logger base_device::get_logger() const { return m_logger; }
rclcpp::Clock::SharedPtr base_device::get_clock() const { return m_clock; }

std::shared_ptr<clover2_common::node_context> base_device::get_node_context()
    const {
    return m_node_context;
}

}  // namespace clover2_display::device
