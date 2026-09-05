#include <clover2_common/node_context.hpp>
#include <clover2_display/client.hpp>
#include <clover2_notification/data/priority.hpp>
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
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace clover2_notification::outputs {

namespace {

constexpr int k_default_width = 128;
constexpr int k_default_height = 64;

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
 * @brief Notification output that renders status rows to a clover2_display
 * device.
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
        if (m_alert_timer) {
            m_alert_timer->cancel();
            m_alert_timer.reset();
        }

        {
            std::lock_guard<std::mutex> lock(m_status_mutex);
            m_alert_active = false;
            m_invert_screen = false;
        }
        output::clear();
        render_and_send();
    }

private:
    /** @brief Text style used by all fields in the display layout. */
    struct text_style {
        /** @brief OpenCV Hershey font scale. */
        double scale{0.30};

        /** @brief OpenCV text stroke thickness in pixels. */
        int thickness{1};
    };

    /** @brief Configurable geometry and style of the status screen. */
    struct layout_config {
        /** @brief Left and right screen margins in pixels. */
        int margin_left{2};
        int margin_right{2};

        /** @brief Shared style of title and status text. */
        text_style font;

        /** @brief Title text and its baseline position in pixels. */
        std::string title{"Clover2"};
        int title_baseline_y{9};

        /** @brief Geometry of the vertically stacked status rows. */
        int statuses_first_baseline_y{20};
        int statuses_line_height{10};
    };

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

        /** @brief Child statuses rendered on one row by "group" rows. */
        std::vector<std::string> items;

        /** @brief Separator inserted between child statuses in a group row. */
        std::string separator{" "};
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
        load_layout_config();
        load_status_configs();
        m_alert_enabled =
            declare_output_parameter<bool>("alert.enabled", m_alert_enabled);
        m_alert_invert_period = declare_output_parameter<double>(
            "alert.invert_period", m_alert_invert_period);

        if (m_refresh_period <= 0.0) {
            throw std::invalid_argument(
                "Display refresh_period should be positive");
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

        RCLCPP_INFO(m_logger,
                    "Display output initialized: base_path='%s' "
                    "refresh_period=%.2fs title='%s' statuses=%zu alert=%s "
                    "invert_period=%.2fs",
                    m_base_path.c_str(), m_refresh_period,
                    m_layout.title.c_str(), m_status_names.size(),
                    m_alert_enabled ? "enabled" : "disabled",
                    m_alert_invert_period);

        render_and_send();
    }

    /** @brief Load and validate the configurable status-screen layout. */
    void load_layout_config() {
        m_layout.margin_left = declare_output_parameter<int>(
            "layout.margin.left", m_layout.margin_left);
        m_layout.margin_right = declare_output_parameter<int>(
            "layout.margin.right", m_layout.margin_right);
        m_layout.font.scale = declare_output_parameter<double>(
            "layout.font.scale", m_layout.font.scale);
        m_layout.font.thickness = declare_output_parameter<int>(
            "layout.font.thickness", m_layout.font.thickness);
        m_layout.title = declare_output_parameter<std::string>(
            "layout.title.text", m_layout.title);
        m_layout.title_baseline_y = declare_output_parameter<int>(
            "layout.title.baseline_y", m_layout.title_baseline_y);
        m_layout.statuses_first_baseline_y =
            declare_output_parameter<int>("layout.statuses.first_baseline_y",
                                          m_layout.statuses_first_baseline_y);
        m_layout.statuses_line_height = declare_output_parameter<int>(
            "layout.statuses.line_height", m_layout.statuses_line_height);

        if (m_layout.margin_left < 0 || m_layout.margin_right < 0 ||
            m_layout.font.scale <= 0.0 || m_layout.font.thickness <= 0 ||
            m_layout.title_baseline_y < 0 ||
            m_layout.statuses_first_baseline_y < 0 ||
            m_layout.statuses_line_height <= 0) {
            throw std::invalid_argument("Invalid display layout configuration");
        }
    }

    /** @brief Load configured status rows and event mappings. */
    void load_status_configs() {
        m_status_names = declare_output_parameter<std::vector<std::string>>(
            "status_names", m_status_names);

        m_status_configs.clear();
        m_status_states.clear();
        m_event_to_status.clear();

        std::vector<std::string> loading;
        for (const auto& status_name : m_status_names) {
            load_status_config(status_name, loading);
        }
    }

    /** @brief Load one status config and any child configs used by groups. */
    void load_status_config(const std::string& status_name,
                            std::vector<std::string>& loading) {
        if (m_status_configs.contains(status_name)) {
            return;
        }
        if (std::find(loading.begin(), loading.end(), status_name) !=
            loading.end()) {
            throw std::invalid_argument("Cyclic display status group: " +
                                        status_name);
        }

        loading.push_back(status_name);
        const auto prefix = "statuses." + status_name;

        status_config config;
        config.event_name = status_name;
        config.label = status_name;
        config.type = declare_output_parameter<std::string>(prefix + ".type",
                                                            config.type);
        config.label = declare_output_parameter<std::string>(prefix + ".label",
                                                             config.label);

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
                                             config.event_name)] = status_name;
            m_status_states.emplace(status_name, status_state{});
        } else if (config.type == "group") {
            config.items = declare_output_parameter<std::vector<std::string>>(
                prefix + ".items", config.items);
            config.separator = declare_output_parameter<std::string>(
                prefix + ".separator", config.separator);

            if (config.items.empty()) {
                throw std::invalid_argument(
                    "Display group status should define non-empty items: " +
                    status_name);
            }
            for (const auto& item : config.items) {
                load_status_config(item, loading);
            }
        } else if (config.type != "hostname") {
            throw std::invalid_argument("Unsupported display status type: " +
                                        config.type);
        }

        loading.pop_back();
        m_status_configs.emplace(status_name, std::move(config));
    }

    /**
     * @brief Process one notification event as a configured status update.
     *
     * @param event Notification event that may update a status row.
     * @param done Completion callback called after processing.
     */
    void process_event(const data::event& event, done_callback done) override {
        if (!m_client) {
            RCLCPP_WARN(m_logger, "Display client is not initialized");
            done();
            return;
        }

        if (update_status_event(event)) {
            render_and_send();
        }

        done();
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
            active =
                m_alert_enabled &&
                std::any_of(m_status_states.begin(), m_status_states.end(),
                            [](const auto& entry) {
                                return entry.second.priority !=
                                       static_cast<int>(data::priority::ok);
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

        const auto period =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
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

        cv::Mat image(height, width, CV_8UC1, cv::Scalar(0));

        render_status(image);

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

    /** @brief Render configured status fields using the configured layout. */
    void render_status(cv::Mat& image) const {
        draw_text(image, m_layout.title, m_layout.margin_left,
                  m_layout.title_baseline_y, 1);

        int y = m_layout.statuses_first_baseline_y;
        for (const auto& status_name : m_status_names) {
            if (y >= image.rows - 1) {
                break;
            }

            const int lines_drawn = draw_text(image, format_status(status_name),
                                              m_layout.margin_left, y);
            y += lines_drawn * m_layout.statuses_line_height;
        }
    }

    /**
     * @brief Draw word-wrapped non-antialiased text for a monochrome display.
     *
     * @return Number of rendered lines.
     */
    int draw_text(cv::Mat& image, const std::string& text, int x, int y,
                  int max_lines = -1) const {
        const int max_width = image.cols - x - m_layout.margin_right;
        const auto lines = wrap_text(text, max_width);
        int lines_drawn{};
        for (const auto& line : lines) {
            if ((max_lines >= 0 && lines_drawn >= max_lines) ||
                y >= image.rows - 1) {
                break;
            }

            cv::putText(image, line, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX,
                        m_layout.font.scale, cv::Scalar(255),
                        m_layout.font.thickness, cv::LINE_8);
            ++lines_drawn;
            y += m_layout.statuses_line_height;
        }

        return lines_drawn;
    }

    /** @brief Wrap text to lines whose rendered width fits @p max_width. */
    std::vector<std::string> wrap_text(const std::string& text,
                                       int max_width) const {
        std::vector<std::string> lines;
        if (max_width <= 0) {
            return lines;
        }

        const auto fits = [this, max_width](const std::string& candidate) {
            return cv::getTextSize(candidate, cv::FONT_HERSHEY_SIMPLEX,
                                   m_layout.font.scale, m_layout.font.thickness,
                                   nullptr)
                       .width <= max_width;
        };

        std::string line;
        size_t word_start{};
        while (word_start < text.size()) {
            const auto word_end = text.find(' ', word_start);
            const auto word = text.substr(word_start, word_end - word_start);
            word_start =
                word_end == std::string::npos ? text.size() : word_end + 1;
            if (word.empty()) {
                continue;
            }

            const auto candidate = line.empty() ? word : line + " " + word;
            if (fits(candidate)) {
                line = candidate;
                continue;
            }

            if (!line.empty()) {
                lines.push_back(std::move(line));
                line.clear();
            }

            const auto parts = split_word(word, max_width);
            for (size_t index{}; index < parts.size(); ++index) {
                if (index + 1 == parts.size() && fits(parts[index])) {
                    line = parts[index];
                } else {
                    lines.push_back(parts[index]);
                }
            }
        }

        if (!line.empty()) {
            lines.push_back(std::move(line));
        }

        return lines;
    }

    /** @brief Split an overlong word into rendered-width-limited parts. */
    std::vector<std::string> split_word(const std::string& word,
                                        int max_width) const {
        std::vector<std::string> parts;
        std::string part;
        for (const char character : word) {
            const auto candidate = part + character;
            if (!part.empty() &&
                cv::getTextSize(candidate, cv::FONT_HERSHEY_SIMPLEX,
                                m_layout.font.scale, m_layout.font.thickness,
                                nullptr)
                        .width > max_width) {
                parts.push_back(std::move(part));
                part = character;
            } else {
                part = candidate;
            }
        }

        if (!part.empty()) {
            parts.push_back(std::move(part));
        }

        return parts;
    }

    /** @brief Format one configured status row. */
    std::string format_status(const std::string& status_name) const {
        const auto config_it = m_status_configs.find(status_name);
        if (config_it == m_status_configs.end()) {
            return status_name + ": n/a";
        }

        const auto& config = config_it->second;
        if (config.type == "hostname") {
            return format_labeled_message(config.label, get_hostname());
        }
        if (config.type == "group") {
            std::string result;
            for (const auto& item : config.items) {
                if (!result.empty()) {
                    result += config.separator;
                }
                result += format_status(item);
            }
            return result;
        }

        std::lock_guard<std::mutex> lock(m_status_mutex);
        const auto state_it = m_status_states.find(status_name);
        if (state_it == m_status_states.end()) {
            return format_labeled_message(config.label, "n/a");
        }

        return format_labeled_message(config.label, state_it->second.message);
    }

    /** @brief Format status message with optional label prefix. */
    static std::string format_labeled_message(const std::string& label,
                                              const std::string& message) {
        if (label.empty()) {
            return message;
        }

        return label + ": " + message;
    }

    std::string m_base_path{"display"};
    double m_refresh_period{1.0};
    layout_config m_layout;
    std::vector<std::string> m_status_names{"hostname", "network", "cpu",
                                            "temperature"};
    std::unordered_map<std::string, status_config> m_status_configs;
    std::unordered_map<std::string, status_state> m_status_states;
    std::unordered_map<std::string, std::string> m_event_to_status;
    bool m_alert_enabled{true};
    double m_alert_invert_period{0.5};
    bool m_alert_active{false};
    bool m_invert_screen{false};

    std::shared_ptr<clover2_display::client> m_client;
    rclcpp::TimerBase::SharedPtr m_refresh_timer;
    rclcpp::TimerBase::SharedPtr m_alert_timer;
    std::mutex m_render_mutex;
    mutable std::mutex m_status_mutex;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_display_output")};
};

}  // namespace clover2_notification::outputs

PLUGINLIB_EXPORT_CLASS(clover2_notification::outputs::display,
                       clover2_notification::output)
