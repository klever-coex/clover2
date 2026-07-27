#include <clover2_common/util/parameter.hpp>
#include <clover2_display/device/base_device.hpp>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <cstdint>
#include <fcntl.h>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

constexpr uint32_t k_width = 128;
constexpr uint32_t k_height = 64;
constexpr uint8_t k_i2c_address = 0x3C;
constexpr size_t k_page_buffer_size = k_width * k_height / 8;
constexpr double k_max_fps = 10.0;

constexpr uint8_t k_control_command = 0x00;
constexpr uint8_t k_control_data = 0x40;

}  // namespace

namespace clover2_display::device {

class ssd1306_i2c : public base_device {
public:
    ssd1306_i2c() = default;
    ~ssd1306_i2c() override {
        if (m_i2c_fd >= 0) {
            close(m_i2c_fd);
            m_i2c_fd = -1;
        }
    }

protected:
    void on_initialize() override {
        info().width = k_width;
        info().height = k_height;
        info().max_fps = k_max_fps;
        info().color_model = "monochrome";
        info().supported_encodings = {"mono8"};

        if (m_i2c_fd >= 0) {
            close(m_i2c_fd);
            m_i2c_fd = -1;
        }

        auto parameters_interface =
            get_node_context()->get_node_parameters_interface();

        clover2_common::util::declare_parameter_if_not_declared(
            parameters_interface, get_name() + ".i2c_device", "/dev/i2c-1");

        rclcpp::Parameter device_p;
        parameters_interface->get_parameter(get_name() + ".i2c_device",
                                            device_p);

        const auto i2c_device = device_p.as_string();

        m_i2c_fd = open(i2c_device.c_str(), O_RDWR);
        if (m_i2c_fd < 0) {
            throw std::runtime_error("ssd1306_i2c: failed to open " +
                                     i2c_device);
        }

        if (ioctl(m_i2c_fd, I2C_SLAVE, k_i2c_address) < 0) {
            close(m_i2c_fd);
            m_i2c_fd = -1;
            throw std::runtime_error(
                "ssd1306_i2c: failed to select I2C address");
        }

        m_page_buffer.assign(k_page_buffer_size, 0x00);

        initialize_display();
        clear_display();

        RCLCPP_INFO(get_logger(), "ssd1306_i2c initialized: %ux%u at %s/0x%02X",
                    k_width, k_height, i2c_device.c_str(), k_i2c_address);
    }

    void on_cleanup() override {
        if (m_i2c_fd >= 0) {
            clear_display();
            close(m_i2c_fd);
            m_i2c_fd = -1;
        }

        m_page_buffer.clear();
    }

    void write_raw_frame(
        const clover2_display::data::display_frame& frame) override {
        if (m_i2c_fd < 0) {
            throw std::runtime_error("ssd1306_i2c: device not initialized");
        }

        pack_mono8_to_page_buffer(frame);
        flush();
    }

private:
    void initialize_display() {
        write_commands({
            0xAE,        // Display off
            0x20, 0x00,  // Horizontal addressing mode
            0xB0,        // Page start address
            0xC8,        // COM output scan direction remapped
            0x00,        // Low column address
            0x10,        // High column address
            0x40,        // Start line address
            0x81, 0x7F,  // Contrast
            0xA1,        // Segment remap
            0xA6,        // Normal display
            0xA8, 0x3F,  // Multiplex ratio for 128x64
            0xA4,        // Display follows RAM
            0xD3, 0x00,  // Display offset
            0xD5, 0x80,  // Display clock divide ratio
            0xD9, 0xF1,  // Pre-charge period
            0xDA, 0x12,  // COM pins hardware config for 128x64
            0xDB, 0x40,  // VCOMH deselect level
            0x8D, 0x14,  // Charge pump enabled
            0xAF,        // Display on
        });
    }

    void clear_display() {
        if (m_i2c_fd < 0) {
            return;
        }

        if (m_page_buffer.size() != k_page_buffer_size) {
            m_page_buffer.assign(k_page_buffer_size, 0x00);
        } else {
            std::fill(m_page_buffer.begin(), m_page_buffer.end(), 0x00);
        }

        flush();
    }

    void pack_mono8_to_page_buffer(
        const clover2_display::data::display_frame& frame) {
        std::fill(m_page_buffer.begin(), m_page_buffer.end(), 0x00);

        for (uint32_t y = 0; y < k_height; ++y) {
            const auto* row = frame.image.ptr<uint8_t>(static_cast<int>(y));
            for (uint32_t x = 0; x < k_width; ++x) {
                const auto pixel = row[x];
                if (pixel > 0) {
                    const auto page = y / 8;
                    const auto bit = y % 8;
                    const auto offset = page * k_width + x;
                    m_page_buffer[offset] |= static_cast<uint8_t>(1u << bit);
                }
            }
        }
    }

    void flush() {
        write_commands({
            0x21, 0x00, static_cast<uint8_t>(k_width - 1),  // Column address
            0x22, 0x00, static_cast<uint8_t>((k_height / 8) - 1),  // Page addr
        });

        write_data(m_page_buffer);
    }

    void write_command(uint8_t command) {
        const uint8_t buffer[2] = {k_control_command, command};
        write_all(buffer, sizeof(buffer));
    }

    void write_commands(std::initializer_list<uint8_t> commands) {
        for (const auto command : commands) {
            write_command(command);
        }
    }

    void write_data(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> buffer;
        buffer.reserve(data.size() + 1);
        buffer.push_back(k_control_data);
        buffer.insert(buffer.end(), data.begin(), data.end());

        write_all(buffer.data(), buffer.size());
    }

    void write_all(const uint8_t* data, size_t size) {
        size_t written = 0;
        while (written < size) {
            const auto ret = ::write(m_i2c_fd, data + written, size - written);
            if (ret < 0) {
                throw std::runtime_error("ssd1306_i2c: I2C write failed");
            }

            if (ret == 0) {
                throw std::runtime_error(
                    "ssd1306_i2c: I2C write returned zero bytes");
            }

            written += static_cast<size_t>(ret);
        }
    }

    int m_i2c_fd{-1};
    std::vector<uint8_t> m_page_buffer{};
};

}  // namespace clover2_display::device

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_display::device::ssd1306_i2c,
                       clover2_display::device::base_device)
