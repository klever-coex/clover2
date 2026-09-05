#pragma once

// clover2
#include <clover2_led/data/color.hpp>
#include <clover2_led/data/driver_info.hpp>
#include <clover2_led/data/led_frame.hpp>

// ROS2
#include <clover2_led_msgs/msg/color.hpp>
#include <clover2_led_msgs/msg/led_frame.hpp>
#include <clover2_led_msgs/srv/get_current_frame.hpp>
#include <clover2_led_msgs/srv/get_driver_info.hpp>
#include <clover2_led_msgs/srv/start_animation.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <chrono>
#include <stdexcept>
#include <string>

namespace clover2_led {

class client {
public:
    template <typename NodeT>
    explicit client(const NodeT& node, const std::string& base_path = "",
                    rclcpp::CallbackGroup::SharedPtr cb_group = nullptr)
        : m_logger(node->get_logger().get_child("led_client"))
        , m_base_path(base_path)
        , m_valid(false)
        , m_led_count(0) {
        rclcpp::PublisherOptions pub_options;
        pub_options.callback_group = cb_group;

        auto node_parameters = node->get_node_parameters_interface();
        auto node_topics = node->get_node_topics_interface();

        m_frame_pub = rclcpp::create_publisher<clover2_led_msgs::msg::LedFrame>(
            node_parameters, node_topics, make_topic("led_frame"),
            rclcpp::SystemDefaultsQoS(), pub_options);

        m_get_info_client =
            rclcpp::create_client<clover2_led_msgs::srv::GetDriverInfo>(
                node->get_node_base_interface(),
                node->get_node_graph_interface(),
                node->get_node_services_interface(),
                make_service("get_driver_info"), rclcpp::ServicesQoS(),
                cb_group);

        m_get_frame_client =
            rclcpp::create_client<clover2_led_msgs::srv::GetCurrentFrame>(
                node->get_node_base_interface(),
                node->get_node_graph_interface(),
                node->get_node_services_interface(),
                make_service("get_current_frame"), rclcpp::ServicesQoS(),
                cb_group);

        m_start_animation_client =
            rclcpp::create_client<clover2_led_msgs::srv::StartAnimation>(
                node->get_node_base_interface(),
                node->get_node_graph_interface(),
                node->get_node_services_interface(),
                make_service("start_animation"), rclcpp::ServicesQoS(),
                cb_group);

        update_driver_info();
    }

    bool valid() const { return m_valid; }

    int get_led_count() const { return m_led_count; }

    double get_max_fps() const { return m_driver_info.max_fps; }

    data::led_frame get_current_frame() {
        if (!m_get_frame_client->wait_for_service(
                std::chrono::milliseconds(1000))) {
            throw std::runtime_error(
                std::string(m_get_frame_client->get_service_name()) +
                " service is not available!");
        }

        auto request =
            std::make_shared<clover2_led_msgs::srv::GetCurrentFrame::Request>();
        auto future = m_get_frame_client->async_send_request(request);

        auto status = future.wait_for(std::chrono::milliseconds(1000));
        if (status != std::future_status::ready) {
            throw std::runtime_error("Timeout waiting for get_current_frame");
        }

        auto resp = future.get();
        if (!resp->success) {
            throw std::runtime_error("get_current_frame failed: " +
                                     resp->message);
        }

        data::led_frame frame;
        frame.brightness = resp->brightness;
        frame.pixels.reserve(resp->colors.size());
        for (const auto& c : resp->colors) {
            frame.pixels.emplace_back(c);
        }

        return frame;
    }

    void send_frame(const data::led_frame& frame) {
        m_frame_pub->publish(frame.to_msg());
    }

    void send_frame(const clover2_led_msgs::msg::LedFrame& msg) {
        m_frame_pub->publish(msg);
    }

    void clear() {
        send_frame(data::led_frame::filled(data::color{0, 0, 0}, m_led_count));
    }

    void fill(const data::color& color) {
        send_frame(data::led_frame::filled(color, m_led_count));
    }

    void solid_color(const data::color& color, float brightness = 1.0F,
                     float duration = 0.0F) {
        auto req =
            std::make_shared<clover2_led_msgs::srv::StartAnimation::Request>();
        req->animation_name = "solid_color";
        req->brightness = brightness;
        req->duration = duration;
        req->colors.push_back(color.to_msg());
        call_animation(req);
    }

    void blink(const data::color& color, float period = 1.0F,
               float brightness = 1.0F, float duration = 0.0F) {
        auto req =
            std::make_shared<clover2_led_msgs::srv::StartAnimation::Request>();
        req->animation_name = "blink";
        req->brightness = brightness;
        req->period = period;
        req->duration = duration;
        req->colors.push_back(color.to_msg());
        call_animation(req);
    }

    void rainbow(float period = 2.0F, float brightness = 1.0F,
                 float duration = 0.0F) {
        auto req =
            std::make_shared<clover2_led_msgs::srv::StartAnimation::Request>();
        req->animation_name = "rainbow";
        req->brightness = brightness;
        req->period = period;
        req->duration = duration;
        call_animation(req);
    }

    rclcpp::Publisher<clover2_led_msgs::msg::LedFrame>::SharedPtr
    get_publisher() const {
        return m_frame_pub;
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

    void call_animation(
        std::shared_ptr<clover2_led_msgs::srv::StartAnimation::Request> req) {
        if (!m_start_animation_client->wait_for_service(
                std::chrono::milliseconds(1000))) {
            throw std::runtime_error("start_animation service not available");
        }

        auto future = m_start_animation_client->async_send_request(req);
        auto status = future.wait_for(std::chrono::milliseconds(1000));
        if (status != std::future_status::ready) {
            throw std::runtime_error("Timeout waiting for start_animation");
        }

        auto resp = future.get();
        if (!resp->success) {
            throw std::runtime_error("start_animation failed: " +
                                     resp->message);
        }
    }

    void update_driver_info() {
        if (!m_get_info_client->wait_for_service(
                std::chrono::milliseconds(1000))) {
            throw std::runtime_error(
                std::string(m_get_info_client->get_service_name()) +
                " service is not available!");
        }

        auto request =
            std::make_shared<clover2_led_msgs::srv::GetDriverInfo::Request>();
        m_get_info_client->async_send_request(
            request,
            [this](rclcpp::Client<
                   clover2_led_msgs::srv::GetDriverInfo>::SharedFuture future) {
                if (!future.valid()) {
                    RCLCPP_ERROR(m_logger, "Failed to get driver info");
                    return;
                }

                auto resp = future.get();
                if (!resp->success) {
                    RCLCPP_ERROR(m_logger, "get_driver_info failed: %s",
                                 resp->message.c_str());
                    return;
                }

                RCLCPP_INFO(m_logger, "Driver info: led_count=%d, max_fps=%.1f",
                            resp->led_count, resp->max_fps);

                m_driver_info.led_count = resp->led_count;
                m_driver_info.max_fps = resp->max_fps;
                m_led_count = static_cast<int>(resp->led_count);
                m_valid = true;
            });
    }

    rclcpp::Logger m_logger;
    std::string m_base_path;

    rclcpp::Publisher<clover2_led_msgs::msg::LedFrame>::SharedPtr m_frame_pub;
    rclcpp::Client<clover2_led_msgs::srv::GetDriverInfo>::SharedPtr
        m_get_info_client;
    rclcpp::Client<clover2_led_msgs::srv::GetCurrentFrame>::SharedPtr
        m_get_frame_client;
    rclcpp::Client<clover2_led_msgs::srv::StartAnimation>::SharedPtr
        m_start_animation_client;

    bool m_valid;
    data::driver_info m_driver_info;
    int m_led_count;
};

}  // namespace clover2_led
