#include <clover2_common/node_context.hpp>
#include <clover2_display/client.hpp>
#include <clover2_notification/output.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <pluginlib/class_list_macros.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace clover2_notification::outputs {

namespace {

constexpr int k_default_width = 128;
constexpr int k_default_height = 64;
constexpr int k_margin = 2;
constexpr int k_line_height = 10;
constexpr uint8_t k_on = 255;
constexpr uint8_t k_off = 0;

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

std::string make_event_key(const std::string& source, const std::string& name) {
    return source + "\n" + name;
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
        if (m_alert_timer) {
            m_alert_timer->cancel();
            m_alert_timer.reset();
        }

        {
            std::lock_guard<std::mutex> lock(m_status_mutex);
            m_alert_active = false;
            m_invert_screen = false;
        }
        m_active_event.reset();
        output::clear();
        render_and_send();
    }

private:
    /** @brief Configuration of one rendered status row. */
    struct status_config {
        /** @brief Row type. "event" rows are updated by notification events. */
        std::string type{"event"};

        /** @brief Event source matched by event rows. */
        std::string source{"system"};

        /** @brief Event name matched by event rows. */
        std::string event_name;

        /** @brief Display label rendered before the status message. */
        std::string label;
    };

    /** @brief Runtime state of one event-backed status row. */
    struct status_state {
        /** @brief Last received event priority. */
        int priority{};

        /** @brief Last received event message. */
        std::string message{"n/a"};
    };

    /**
     * @brief Initialize display client, read local parameters and start status
     * refresh timer.
     *
     */
    void on_initialize() override {
        m_logger = node_context()
                       ->get_logger()
                       .get_child("display_output")
                       .get_child(id());

        m_base_path =
            declare_output_parameter<std::string>("base_path", m_base_path);
        m_refresh_period = declare_output_parameter<double>("refresh_period",
                                                            m_refresh_period);
        m_layout = declare_output_parameter<std::string>("layout", m_layout);
        load_status_configs();
        m_alert_enabled = declare_output_parameter<bool>("alert.enabled",
                                                         m_alert_enabled);
        m_alert_invert_period = declare_output_parameter<double>(
            "alert.invert_period", m_alert_invert_period);
        m_overlay_enabled = declare_output_parameter<bool>(
            "notification_overlay.enabled", m_overlay_enabled);
        m_overlay_duration = declare_output_parameter<double>(
            "notification_overlay.duration", m_overlay_duration);

        if (m_refresh_period <= 0.0) {
            throw std::invalid_argument(
                "Display refresh_period should be positive");
        }
        if (m_overlay_duration <= 0.0) {
            throw std::invalid_argument(
                "Display notification_overlay.duration should be positive");
        }
        if (m_alert_invert_period <= 0.0) {
            throw std::invalid_argument(
                "Display alert.invert_period should be positive");
        }

        m_client = std::make_shared<clover2_display::client>(node_context(),
                                                             m_base_path);

        const auto refresh_period =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(m_refresh_period));
        m_refresh_timer =
            create_timer(refresh_period, [this]() { render_and_send(); });

        RCLCPP_INFO(
            m_logger,
            "Display output initialized: base_path='%s' "
            "refresh_period=%.2fs layout='%s' statuses=%zu alert=%s "
            "invert_period=%.2fs overlay=%s duration=%.2fs",
            m_base_path.c_str(), m_refresh_period, m_layout.c_str(),
            m_status_names.size(), m_alert_enabled ? "enabled" : "disabled",
            m_alert_invert_period, m_overlay_enabled ? "enabled" : "disabled",
            m_overlay_duration);

        render_and_send();
    }

    /** @brief Load configured status rows and event mappings. */
    void load_status_configs() {
        m_status_names = declare_output_parameter<std::vector<std::string>>(
            "status_names", m_status_names);

        m_status_configs.clear();
        m_status_states.clear();
        m_event_to_status.clear();

        for (const auto& status_name : m_status_names) {
            const auto prefix = "statuses." + status_name;

            status_config config;
            config.event_name = status_name;
            config.label = status_name;
            config.type = declare_output_parameter<std::string>(prefix + ".type",
                                                                config.type);
            config.label = declare_output_parameter<std::string>(
                prefix + ".label", config.label);

            if (config.type == "event") {
                config.source = declare_output_parameter<std::string>(
                    prefix + ".source", config.source);
                config.event_name = declare_output_parameter<std::string>(
                    prefix + ".event_name", config.event_name);

                if (config.source.empty() || config.event_name.empty()) {
                    throw std::invalid_argument(
                        "Display event status should define non-empty source "
                        "and event_name: " +
                        status_name);
                }

                m_event_to_status[make_event_key(config.source,
                                                 config.event_name)] =
                    status_name;
                m_status_states.emplace(status_name, status_state{});
            } else if (config.type != "hostname") {
                throw std::invalid_argument("Unsupported display status type: " +
                                            config.type);
            }

            m_status_configs.emplace(status_name, std::move(config));
        }
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

        if (update_status_event(event)) {
            render_and_send();
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
        m_overlay_timer = create_timer(duration, [this, done]() {
            if (m_overlay_timer) {
                m_overlay_timer->cancel();
                m_overlay_timer.reset();
            }

            m_active_event.reset();
            render_and_send();
            done();
        });
    }

    /** @brief Update configured status row from matching event. */
    bool update_status_event(const data::event& event) {
        const auto status_it =
            m_event_to_status.find(make_event_key(event.source, event.name));
        if (status_it == m_event_to_status.end()) {
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(m_status_mutex);
            auto& state = m_status_states[status_it->second];
            state.priority = event.priority;
            state.message = event.message;
        }

        update_alert_state();
        return true;
    }

    /** @brief Start or stop screen inversion according to status priorities. */
    void update_alert_state() {
        bool active{};
        {
            std::lock_guard<std::mutex> lock(m_status_mutex);
            active = m_alert_enabled &&
                     std::any_of(m_status_states.begin(), m_status_states.end(),
                                 [](const auto& entry) {
                                     return entry.second.priority != 0;
                                 });
            if (active == m_alert_active) {
                return;
            }

            m_alert_active = active;
            m_invert_screen = false;
        }

        if (!active) {
            if (m_alert_timer) {
                m_alert_timer->cancel();
                m_alert_timer.reset();
            }
            return;
        }

        const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(m_alert_invert_period));
        m_alert_timer = create_timer(period, [this]() {
            {
                std::lock_guard<std::mutex> lock(m_status_mutex);
                m_invert_screen = !m_invert_screen;
            }
            render_and_send();
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
        bool invert_screen{};
        {
            std::lock_guard<std::mutex> lock(m_status_mutex);
            invert_screen = m_alert_active && m_invert_screen;
        }

        if (invert_screen) {
            cv::bitwise_not(image, image);
        }

        sensor_msgs::msg::Image msg;
        msg.header.stamp =
            node_context()
                ? node_context()->get_node_clock_interface()->get_clock()->now()
                : rclcpp::Clock().now();
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
            RCLCPP_ERROR(m_logger, "Failed to send display image: %s",
                         e.what());
        }
    }

    /** @brief Render configured status fields in compact layout. */
    void render_status(cv::Mat& image) const {
        draw_text(image, "Clover2", k_margin, 9);

        int y = 20;
        for (const auto& status_name : m_status_names) {
            if (y >= image.rows - 1) {
                break;
            }

            draw_text(image, format_status(status_name), k_margin, y);
            y += k_line_height;
        }
    }

    /** @brief Render active notification overlay. */
    void render_overlay(cv::Mat& image, const data::event& event) const {
        image.setTo(cv::Scalar(k_on));

        draw_text(
            image,
            source_label(event.source) + " " + priority_label(event.priority),
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

    /** @brief Format one configured status row. */
    std::string format_status(const std::string& status_name) const {
        const auto config_it = m_status_configs.find(status_name);
        if (config_it == m_status_configs.end()) {
            return status_name + ": n/a";
        }

        const auto& config = config_it->second;
        if (config.type == "hostname") {
            return config.label + ": " + get_hostname();
        }

        std::lock_guard<std::mutex> lock(m_status_mutex);
        const auto state_it = m_status_states.find(status_name);
        if (state_it == m_status_states.end()) {
            return config.label + ": n/a";
        }

        return config.label + ": " + state_it->second.message;
    }

    std::string m_base_path{"display"};
    double m_refresh_period{1.0};
    std::string m_layout{"compact"};
    std::vector<std::string> m_status_names{"hostname", "network", "cpu",
                                            "temperature"};
    std::unordered_map<std::string, status_config> m_status_configs;
    std::unordered_map<std::string, status_state> m_status_states;
    std::unordered_map<std::string, std::string> m_event_to_status;
    bool m_alert_enabled{true};
    double m_alert_invert_period{0.5};
    bool m_alert_active{false};
    bool m_invert_screen{false};
    bool m_overlay_enabled{true};
    double m_overlay_duration{3.0};

    std::shared_ptr<clover2_display::client> m_client;
    rclcpp::TimerBase::SharedPtr m_refresh_timer;
    rclcpp::TimerBase::SharedPtr m_overlay_timer;
    rclcpp::TimerBase::SharedPtr m_alert_timer;
    std::optional<data::event> m_active_event;
    std::mutex m_render_mutex;
    mutable std::mutex m_status_mutex;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_display_output")};
};

}  // namespace clover2_notification::outputs

PLUGINLIB_EXPORT_CLASS(clover2_notification::outputs::display,
                       clover2_notification::output)
