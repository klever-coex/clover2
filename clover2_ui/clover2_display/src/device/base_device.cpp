#include <clover2_display/device/base_device.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>

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
}

void base_device::cleanup() noexcept {
    on_cleanup();
    m_node_context.reset();
}

const data::display_info& base_device::info() const noexcept { return m_info; }
data::display_info& base_device::info() noexcept { return m_info; }

void base_device::write(const data::display_frame& frame) {
    if (frame.image.empty()) {
        throw std::runtime_error("display: empty frame image");
    }

    if (std::find(m_info.supported_encodings.begin(),
                  m_info.supported_encodings.end(),
                  frame.encoding) == m_info.supported_encodings.end()) {
        throw std::runtime_error("display: unsupported frame encoding '" +
                                 frame.encoding + "'");
    }

    if (frame.width() != static_cast<int>(m_info.width) ||
        frame.height() != static_cast<int>(m_info.height)) {
        throw std::runtime_error(
            "display: unsupported frame size " + std::to_string(frame.width()) +
            "x" + std::to_string(frame.height()) + ", expected " +
            std::to_string(m_info.width) + "x" + std::to_string(m_info.height));
    }

    if (frame.image.type() != data::encoding_to_cv_type(frame.encoding)) {
        throw std::runtime_error(
            "display: image matrix type does not match "
            "frame encoding");
    }

    write_raw_frame(frame);
}

const std::string& base_device::get_name() const { return m_name; }

rclcpp::Logger base_device::get_logger() const { return m_logger; }
rclcpp::Clock::SharedPtr base_device::get_clock() const { return m_clock; }

std::shared_ptr<clover2_common::node_context> base_device::get_node_context()
    const {
    return m_node_context;
}

}  // namespace clover2_display::device
