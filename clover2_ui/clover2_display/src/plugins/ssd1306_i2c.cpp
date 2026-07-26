#include <clover2_display/device/base_device.hpp>

namespace clover2_display::device {

class ssd1306_i2c : public base_device {
public:
    ssd1306_i2c() = default;
    ~ssd1306_i2c() override = default;

protected:
    void on_initialize() override {
        // TODO: Open /dev/i2c-1, select address 0x3C and send SSD1306 init
        info().width = 128;
        info().height = 64;
        info().max_fps = 10.0;
        info().color_model = "monochrome";
        info().supported_encodings = {"mono8"};

        RCLCPP_INFO(get_logger(), "ssd1306_i2c placeholder initialized");
    }

    void on_cleanup() override {
        // TODO: Clear display and close I2C file descriptor.
    }

    void write_raw_frame(
        const clover2_display::data::display_frame& /* frame */) override {
        // TODO: Convert mono8 128x64 to SSD1306 page buffer and write over I2C.
    }
};

}  // namespace clover2_display::device

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_display::device::ssd1306_i2c,
                       clover2_display::device::base_device)
