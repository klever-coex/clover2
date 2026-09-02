#include <clover2_display/client.hpp>
#include <clover2_notification/output.hpp>
#include <pluginlib/class_list_macros.hpp>

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/temperature.hpp>
#include <std_msgs/msg/float64.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace clover2_notification::outputs {

namespace {

constexpr int k_default_width = 128;
constexpr int k_default_height = 64;
constexpr int k_margin = 2;
constexpr int k_line_height = 10;
constexpr uint8_t k_on = 255;
constexpr uint8_t k_off = 0;

struct network_status {
    std::string state;
    std::string ipv4;
};

std::string trim_to_width(const std::string& text, int max_chars) {
    if (max_chars <= 0 || static_cast<int>(text.size()) <= max_chars) {
        return text;
    }

    if (max_chars <= 3) {
        return text.substr(0, static_cast<size_t>(max_chars));
    }

    return text.substr(0, static_cast<size_t>(max_chars - 3)) + "...";
}

std::string priority_label(int priority) {
    switch (priority) {
        case 1:
            return "WARN";
        case 2:
            return "ERROR";
        case 3:
            return "STALE";
    }

    return "P" + std::to_string(priority);
}

std::string source_label(std::string_view source) {
    if (source == "system") {
        return "SYS";
    }
    if (source == "diagnostics") {
        return "DIAG";
    }

    return std::string(source);
}

std::string get_hostname() {
    char hostname[256]{};
    if (::gethostname(hostname, sizeof(hostname) - 1) != 0) {
        return "unknown";
    }

    hostname[sizeof(hostname) - 1] = '\0';
    return hostname;
}

}  // namespace

/**
 * @class display
 * @brief Notification output that renders status and notification overlay to a
 * clover2_display device.
 */
class display final : public clover2_notification::output {
public:
    /** @brief Construct a display notification output. */
    display() = default;

    /** @brief Destroy a display notification output. */
    ~display() override = default;

    /** @brief Stop active timers and clear queued notifications. */
    void clear() override {
        if (m_refresh_timer) {
            m_refresh_timer->cancel();
            m_refresh_timer.reset();
        }
        if (m_overlay_timer) {
            m_overlay_timer->cancel();
            m_overlay_timer.reset();
        }

        m_active_event.reset();
        output::clear();
        render_and_send();
    }

private:
    /**
     * @brief Initialize display client, read local parameters and start status
     * refresh timer.
     *
     * @param node Lifecycle node that owns the output plugin.
     */
    void on_initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) override {
        m_node = node;
        m_logger =
            node->get_logger().get_child("display_output").get_child(id());

        m_base_path = node->declare_parameter<std::string>(
            id() + ".base_path", m_base_path);
        m_refresh_period = node->declare_parameter<double>(
            id() + ".refresh_period", m_refresh_period);
        m_layout = node->declare_parameter<std::string>(id() + ".layout",
                                                        m_layout);
        m_fields = node->declare_parameter<std::vector<std::string>>(
            id() + ".fields", m_fields);
        m_interfaces = node->declare_parameter<std::vector<std::string>>(
            id() + ".interfaces", m_interfaces);
        m_cpu_topic = node->declare_parameter<std::string>(
            id() + ".system_topics.cpu", m_cpu_topic);
        m_temperature_topic = node->declare_parameter<std::string>(
            id() + ".system_topics.temperature", m_temperature_topic);
        m_network_topic = node->declare_parameter<std::string>(
            id() + ".system_topics.network", m_network_topic);
        m_overlay_enabled = node->declare_parameter<bool>(
            id() + ".notification_overlay.enabled", m_overlay_enabled);
        m_overlay_duration = node->declare_parameter<double>(
            id() + ".notification_overlay.duration", m_overlay_duration);

        if (m_refresh_period <= 0.0) {
            throw std::invalid_argument(
                "Display refresh_period should be positive");
        }
        if (m_overlay_duration <= 0.0) {
            throw std::invalid_argument(
                "Display notification_overlay.duration should be positive");
        }

        m_client = std::make_shared<clover2_display::client>(node, m_base_path);

        const auto status_qos =
            rclcpp::QoS(rclcpp::KeepLast(1)).reliable().transient_local();
        m_cpu_subscription = node->create_subscription<std_msgs::msg::Float64>(
            m_cpu_topic, status_qos,
            [this](const std_msgs::msg::Float64& message) {
                {
                    std::lock_guard<std::mutex> lock(m_status_mutex);
                    m_cpu_usage = message.data;
                }
                render_and_send();
            });
        m_temperature_subscription =
            node->create_subscription<sensor_msgs::msg::Temperature>(
                m_temperature_topic, status_qos,
                [this](const sensor_msgs::msg::Temperature& message) {
                    {
                        std::lock_guard<std::mutex> lock(m_status_mutex);
                        m_temperature_celsius = message.temperature;
                    }
                    render_and_send();
                });
        m_network_subscription =
            node->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
                m_network_topic, status_qos,
                [this](const diagnostic_msgs::msg::DiagnosticArray& message) {
                    std::unordered_map<std::string, network_status> statuses;
                    for (const auto& status : message.status) {
                        std::string interface = status.hardware_id;
                        std::string ipv4;
                        for (const auto& value : status.values) {
                            if (value.key == "interface") {
                                interface = value.value;
                            } else if (value.key == "ipv4") {
                                ipv4 = value.value;
                            }
                        }
                        if (!interface.empty()) {
                            statuses.insert_or_assign(
                                interface, network_status{status.message, ipv4});
                        }
                    }
                    {
                        std::lock_guard<std::mutex> lock(m_status_mutex);
                        m_network_statuses = std::move(statuses);
                    }
                    render_and_send();
                });

        const auto refresh_period =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(m_refresh_period));
        m_refresh_timer = m_node->create_wall_timer(
            refresh_period, [this]() { render_and_send(); });

        RCLCPP_INFO(m_logger,
                    "Display output initialized: base_path='%s' "
                    "refresh_period=%.2fs layout='%s' overlay=%s duration=%.2fs",
                    m_base_path.c_str(), m_refresh_period, m_layout.c_str(),
                    m_overlay_enabled ? "enabled" : "disabled",
                    m_overlay_duration);

        render_and_send();
    }

    /**
     * @brief Process one notification event as a temporary display overlay.
     *
     * @param event Notification event to display.
     * @param done Completion callback called when overlay duration expires.
     */
    void process_event(const data::event& event, done_callback done) override {
        if (!m_client) {
            RCLCPP_WARN(m_logger, "Display client is not initialized");
            done();
            return;
        }

        if (!m_overlay_enabled) {
            render_and_send();
            done();
            return;
        }

        RCLCPP_INFO(m_logger,
                    "Show display notification overlay: source='%s' name='%s' "
                    "message='%s' priority=%d queued=%zu output='%s'",
                    event.source.c_str(), event.name.c_str(),
                    event.message.c_str(), event.priority, queued_size(),
                    id().c_str());

        m_active_event = event;
        render_and_send();

        const auto duration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(m_overlay_duration));
        m_overlay_timer = m_node->create_wall_timer(duration, [this, done]() {
            if (m_overlay_timer) {
                m_overlay_timer->cancel();
                m_overlay_timer.reset();
            }

            m_active_event.reset();
            render_and_send();
            done();
        });
    }

    /** @brief Render the current screen state and publish it to display. */
    void render_and_send() {
        std::lock_guard<std::mutex> render_lock(m_render_mutex);

        if (!m_client) {
            return;
        }

        const int width = m_client->valid()
                              ? static_cast<int>(m_client->get_width())
                              : k_default_width;
        const int height = m_client->valid()
                               ? static_cast<int>(m_client->get_height())
                               : k_default_height;

        cv::Mat image(height, width, CV_8UC1, cv::Scalar(k_off));

        if (m_active_event) {
            render_overlay(image, *m_active_event);
        } else {
            render_status(image);
        }

        cv::threshold(image, image, 127, 255, cv::THRESH_BINARY);

        sensor_msgs::msg::Image msg;
        msg.header.stamp = m_node ? m_node->now() : rclcpp::Clock().now();
        msg.header.frame_id = "display";
        msg.height = static_cast<uint32_t>(image.rows);
        msg.width = static_cast<uint32_t>(image.cols);
        msg.encoding = sensor_msgs::image_encodings::MONO8;
        msg.is_bigendian = false;
        msg.step = static_cast<uint32_t>(image.step[0]);
        msg.data.assign(image.datastart, image.dataend);

        try {
            m_client->send_image(msg);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(m_logger, "Failed to send display image: %s", e.what());
        }
    }

    /** @brief Render configured status fields in compact layout. */
    void render_status(cv::Mat& image) const {
        draw_text(image, "Clover2", k_margin, 9);

        int y = 20;
        for (const auto& field : m_fields) {
            if (y >= image.rows - 1) {
                break;
            }

            draw_text(image, format_field(field), k_margin, y);
            y += k_line_height;
        }
    }

    /** @brief Render active notification overlay. */
    void render_overlay(cv::Mat& image, const data::event& event) const {
        image.setTo(cv::Scalar(k_on));

        draw_text(image,
                  source_label(event.source) + " " +
                      priority_label(event.priority),
                  k_margin, 10, k_off);
        draw_text(image, event.name, k_margin, 22, k_off);
        draw_text(image, event.message, k_margin, 36, k_off);
    }

    /** @brief Draw clipped non-antialiased text for a monochrome display. */
    void draw_text(cv::Mat& image, const std::string& text, int x, int y,
                   uint8_t value = k_on) const {
        constexpr double font_scale = 0.30;
        constexpr int thickness = 1;
        const int max_chars = std::max(1, (image.cols - x) / 6);

        cv::putText(image, trim_to_width(text, max_chars), cv::Point(x, y),
                    cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(value),
                    thickness, cv::LINE_8);
    }

    /** @brief Format one configured status field for the MVP status screen. */
    std::string format_field(const std::string& field) const {
        if (field == "hostname") {
            return "host: " + get_hostname();
        }
        if (field == "network") {
            if (m_interfaces.empty()) {
                return "net: n/a";
            }

            std::lock_guard<std::mutex> lock(m_status_mutex);
            std::ostringstream stream;
            stream << "net:";
            for (const auto& interface : m_interfaces) {
                const auto status = m_network_statuses.find(interface);
                if (status == m_network_statuses.end()) {
                    stream << " " << interface << "?";
                } else {
                    stream << " " << interface
                           << (status->second.state == "up" ? "+" : "-");
                }
            }
            return stream.str();
        }
        if (field == "cpu") {
            std::lock_guard<std::mutex> lock(m_status_mutex);
            if (!m_cpu_usage) {
                return "cpu: n/a";
            }
            return "cpu: " +
                   std::to_string(static_cast<int>(std::lround(*m_cpu_usage))) +
                   "%";
        }
        if (field == "temperature") {
            std::lock_guard<std::mutex> lock(m_status_mutex);
            if (!m_temperature_celsius) {
                return "temp: n/a";
            }
            return "temp: " + std::to_string(
                                 static_cast<int>(std::lround(*m_temperature_celsius))) +
                   "C";
        }

        return field + ": n/a";
    }

    std::string m_base_path{"display"};
    double m_refresh_period{1.0};
    std::string m_layout{"compact"};
    std::vector<std::string> m_fields{"hostname", "network", "cpu",
                                      "temperature"};
    std::vector<std::string> m_interfaces;
    std::string m_cpu_topic{"system/cpu"};
    std::string m_temperature_topic{"system/temperature"};
    std::string m_network_topic{"system/network"};
    bool m_overlay_enabled{true};
    double m_overlay_duration{3.0};

    rclcpp_lifecycle::LifecycleNode::SharedPtr m_node;
    std::shared_ptr<clover2_display::client> m_client;
    rclcpp::TimerBase::SharedPtr m_refresh_timer;
    rclcpp::TimerBase::SharedPtr m_overlay_timer;
    rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr m_cpu_subscription;
    rclcpp::Subscription<sensor_msgs::msg::Temperature>::SharedPtr
        m_temperature_subscription;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        m_network_subscription;
    std::optional<data::event> m_active_event;
    std::mutex m_render_mutex;
    mutable std::mutex m_status_mutex;
    std::optional<double> m_cpu_usage;
    std::optional<double> m_temperature_celsius;
    std::unordered_map<std::string, network_status> m_network_statuses;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_display_output")};
};

}  // namespace clover2_notification::outputs

PLUGINLIB_EXPORT_CLASS(clover2_notification::outputs::display,
                       clover2_notification::output)
