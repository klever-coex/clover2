#pragma once

// clover2
#include <clover2_display/data/display_info.hpp>

// ROS2
#include <clover2_display_msgs/srv/get_driver_info.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

// STL
#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

namespace clover2_display {

class client {
public:
    template <typename NodeT>
    explicit client(const NodeT& node, const std::string& base_path = "",
                    rclcpp::CallbackGroup::SharedPtr cb_group = nullptr)
        : m_logger(node->get_logger().get_child("display_client"))
        , m_base_path(base_path)
        , m_valid(false) {
        rclcpp::PublisherOptions pub_options;
        pub_options.callback_group = cb_group;

        m_image_pub = node->template create_publisher<sensor_msgs::msg::Image>(
            make_topic("image"), rclcpp::SystemDefaultsQoS(), pub_options);

        m_get_info_client =
            node->template create_client<clover2_display_msgs::srv::GetDriverInfo>(
                make_service("get_driver_info"), rclcpp::ServicesQoS(),
                cb_group);

        update_driver_info();
    }

    bool valid() const { return m_valid; }

    const data::display_info& get_info() const { return m_display_info; }

    uint32_t get_width() const { return m_display_info.width; }

    uint32_t get_height() const { return m_display_info.height; }

    double get_max_fps() const { return m_display_info.max_fps; }

    const std::vector<std::string>& get_supported_encodings() const {
        return m_display_info.supported_encodings;
    }

    void send_image(const sensor_msgs::msg::Image& msg) {
        m_image_pub->publish(msg);
    }

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr get_publisher()
        const {
        return m_image_pub;
    }

private:
    std::string make_topic(const std::string& name) const {
        if (m_base_path.empty()) {
            return name;
        }
        return m_base_path + "/" + name;
    }

    std::string make_service(const std::string& name) const {
        if (m_base_path.empty()) {
            return name;
        }
        return m_base_path + "/" + name;
    }

    void update_driver_info() {
        if (!m_get_info_client->wait_for_service(
                std::chrono::milliseconds(1000))) {
            throw std::runtime_error(
                std::string(m_get_info_client->get_service_name()) +
                " service is not available!");
        }

        auto request = std::make_shared<
            clover2_display_msgs::srv::GetDriverInfo::Request>();
        m_get_info_client->async_send_request(
            request,
            [this](rclcpp::Client<
                   clover2_display_msgs::srv::GetDriverInfo>::SharedFuture
                       future) {
                if (!future.valid()) {
                    RCLCPP_ERROR(m_logger, "Failed to get display driver info");
                    return;
                }

                auto resp = future.get();
                if (!resp->success) {
                    RCLCPP_ERROR(m_logger, "get_driver_info failed: %s",
                                 resp->message.c_str());
                    return;
                }

                RCLCPP_INFO(m_logger,
                            "Display driver info: %ux%u, max_fps=%.1f",
                            resp->width, resp->height, resp->max_fps);

                m_display_info.width = resp->width;
                m_display_info.height = resp->height;
                m_display_info.max_fps = resp->max_fps;
                m_display_info.supported_encodings = resp->supported_encodings;
                m_valid = true;
            });
    }

    rclcpp::Logger m_logger;
    std::string m_base_path;

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_image_pub;
    rclcpp::Client<clover2_display_msgs::srv::GetDriverInfo>::SharedPtr
        m_get_info_client;

    bool m_valid;
    data::display_info m_display_info;
};

}  // namespace clover2_display
