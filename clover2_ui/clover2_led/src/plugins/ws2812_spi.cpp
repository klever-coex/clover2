#include <clover2_common/util/parameter.hpp>
#include <clover2_led/device/base_device.hpp>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

namespace {

constexpr uint32_t k_spi_speed = 2'400'000;
constexpr uint8_t k_spi_mode = 0;
constexpr uint8_t k_spi_bits = 8;
constexpr size_t k_reset_bytes = 50;
constexpr double k_max_fps = 10.0;

}  // namespace

namespace clover2_led::device {

class ws2812_spi : public base_device {
public:
    ws2812_spi() = default;

    ~ws2812_spi() override {
        if (m_spi_fd >= 0) {
            close(m_spi_fd);
            m_spi_fd = -1;
        }
    }

protected:
    void on_initialize(size_t led_count) override {
        info().max_fps = k_max_fps;

        auto parameters_interface =
            get_node_context()->get_node_parameters_interface();

        clover2_common::util::declare_parameter_if_not_declared(
            parameters_interface, get_name() + ".spi_device", "/dev/spidev0.0");

        rclcpp::Parameter device_p;
        parameters_interface->get_parameter(get_name() + ".spi_device",
                                            device_p);

        std::string device = device_p.as_string();

        m_spi_fd = open(device.c_str(), O_RDWR);
        if (m_spi_fd < 0) {
            throw std::runtime_error("ws2812_spi: failed to open " + device);
        }

        uint8_t mode = k_spi_mode;
        if (ioctl(m_spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
            close(m_spi_fd);
            m_spi_fd = -1;
            throw std::runtime_error("ws2812_spi: failed to set SPI mode");
        }

        uint8_t bits = k_spi_bits;
        if (ioctl(m_spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
            close(m_spi_fd);
            m_spi_fd = -1;
            throw std::runtime_error(
                "ws2812_spi: failed to set SPI bits per word");
        }

        uint32_t speed = k_spi_speed;
        if (ioctl(m_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
            close(m_spi_fd);
            m_spi_fd = -1;
            throw std::runtime_error("ws2812_spi: failed to set SPI speed");
        }

        m_spi_buffer.resize(led_count * 9 + k_reset_bytes, 0x00);

        RCLCPP_INFO(get_logger(), "ws2812_spi: %zu LEDs on %s", led_count,
                    device.c_str());
    }

    void on_cleanup() override {
        if (m_spi_fd >= 0) {
            std::fill(m_spi_buffer.begin(), m_spi_buffer.end(), 0x00);
            transfer();
            close(m_spi_fd);
            m_spi_fd = -1;
        }

        m_spi_buffer.clear();
    }

    void write_raw_frame(
        const std::vector<clover2_led::data::color>& colors) override {
        if (m_spi_fd < 0) {
            throw std::runtime_error("ws2812_spi: device not initialized");
        }

        std::fill(m_spi_buffer.begin() + colors.size() * 9, m_spi_buffer.end(),
                  0x00);

        for (size_t i = 0; i < colors.size(); ++i) {
            set_led(i, colors[i]);
        }

        transfer();
    }

private:
    void set_led(size_t index, const clover2_led::data::color& color) {
        const uint8_t channels[3] = {color.g, color.r, color.b};
        const size_t offset = index * 9;

        for (int c = 0; c < 3; ++c) {
            const uint8_t byte = channels[c];
            uint32_t packet = 0;

            for (int bit = 7; bit >= 0; --bit) {
                packet <<= 3;
                if (byte & (1 << bit)) {
                    packet |= 0b110;
                } else {
                    packet |= 0b100;
                }
            }

            m_spi_buffer[offset + c * 3 + 0] = (packet >> 16) & 0xFF;
            m_spi_buffer[offset + c * 3 + 1] = (packet >> 8) & 0xFF;
            m_spi_buffer[offset + c * 3 + 2] = packet & 0xFF;
        }
    }

    void transfer() {
        struct spi_ioc_transfer tr;
        std::memset(&tr, 0, sizeof(tr));

        tr.tx_buf = reinterpret_cast<unsigned long>(m_spi_buffer.data());
        tr.rx_buf = 0;
        tr.len = m_spi_buffer.size();
        tr.speed_hz = k_spi_speed;
        tr.bits_per_word = k_spi_bits;

        if (ioctl(m_spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
            RCLCPP_ERROR(get_logger(), "ws2812_spi: SPI transfer failed");
        }
    }

    int m_spi_fd{-1};
    std::vector<uint8_t> m_spi_buffer{};
};

}  // namespace clover2_led::device

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_led::device::ws2812_spi,
                       clover2_led::device::base_device)
