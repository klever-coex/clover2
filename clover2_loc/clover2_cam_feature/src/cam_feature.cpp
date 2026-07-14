// clover2
#include <clover2/cam_feature/cam_feature.hpp>
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_common/util/parameter.hpp>

// opencv
#include <cv_bridge/cv_bridge.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <rclcpp/executor.hpp>
#include <rclcpp/logging.hpp>
#include <sensor_msgs/image_encodings.hpp>

// STL
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

const std::vector<std::string> default_plugin_ids = {"aruco"};
const std::vector<std::string> default_plugin_types = {
    "clover2::cam_feature::plugins::aruco"};

}  // namespace

namespace clover2::cam_feature {

cam_feature::cam_feature(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("cam_feature", options) {
    auto diagnostics = std::make_shared<CamFeatureDiagnostics>(
        get_node_base_interface(), get_node_clock_interface(),
        get_node_logging_interface(), get_node_parameters_interface(),
        get_node_timers_interface(), get_node_topics_interface());

    diagnostics->set_diagnostic_callback(
        CamFeatureDiagnostics::diagnostic::camera_info,
        std::bind(&cam_feature::produce_camera_info_diagnostics, this,
                  std::placeholders::_1));
    diagnostics->set_diagnostic_callback(
        CamFeatureDiagnostics::diagnostic::map,
        std::bind(&cam_feature::produce_map_diagnostics, this,
                  std::placeholders::_1));
    diagnostics->set_diagnostic_callback(
        CamFeatureDiagnostics::diagnostic::marker_frequency,
        std::bind(&cam_feature::produce_marker_hz_diagnostics, this,
                  std::placeholders::_1));

    set_node_diagnostics_interface(std::move(diagnostics));

    declare_parameter("feature_plugins", default_plugin_ids);

    for (size_t i = 0; i < default_plugin_ids.size(); i++) {
        declare_parameter(default_plugin_ids[i] + ".plugin",
                          default_plugin_types[i]);
    }

    register_on_configure(
        std::bind(&cam_feature::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&cam_feature::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&cam_feature::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&cam_feature::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&cam_feature::on_shutdown, this, std::placeholders::_1));
}

cam_feature::~cam_feature() = default;

cam_feature::CallbackReturn cam_feature::on_configure(
    const rclcpp_lifecycle::State& state) {
    auto node_context = std::make_shared<clover2_common::node_context>(*this);

    try {
        m_map_client = std::make_shared<clover2::map::client>(this);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Fail to create map: %s", e.what());
        on_cleanup(state);
        return CallbackReturn::FAILURE;
    }

    get_parameter("feature_plugins", m_plugin_ids);
    if (m_plugin_ids == default_plugin_ids) {
        for (size_t i = 0; i < m_plugin_ids.size(); i++) {
            std::string param_name = default_plugin_ids[i] + ".plugin";
            clover2_common::util::declare_parameter_if_not_declared(
                this, param_name, default_plugin_types[i]);
        }
    }

    for (const auto& id : m_plugin_ids) {
        std::string param_name = id + ".plugin";
        if (!has_parameter(param_name)) {
            declare_parameter<std::string>(param_name);
        }

        std::string plugin_type;
        if (!get_parameter(param_name, plugin_type)) {
            RCLCPP_ERROR(get_logger(), "Plugin type for %s not set.",
                         id.c_str());
        }

        try {
            auto plugin = m_plugin_loader.createSharedInstance(plugin_type);
            RCLCPP_INFO(get_logger(), "Created plugin %s with type %s",
                        id.c_str(), plugin_type.c_str());

            plugin->configure(id, node_context, m_map_client);

            m_plugins.insert({id, plugin});
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(), "Fail to load plugin. Exception: %s",
                         e.what());
            on_cleanup(state);
            return CallbackReturn::FAILURE;
        }
    }

    RCLCPP_INFO(get_logger(), "Configure");
    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_activate(
    const rclcpp_lifecycle::State& state) {
    for (const auto& [name, id] : m_plugins) {
        try {
            id->activate();
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(), "Fail to load plugin. Exception: %s",
                         e.what());
            on_deactivate(state);
            return CallbackReturn::FAILURE;
        }
    }

    try {
        m_markers_pub = create_publisher<clover2_pose_msgs::msg::MarkerArray>(
            "~/output/markers", rclcpp::SensorDataQoS());

        m_image_debug_pub = create_publisher<sensor_msgs::msg::Image>(
            "~/output/debug", rclcpp::SystemDefaultsQoS());

        m_camera_info_sub = create_subscription<sensor_msgs::msg::CameraInfo>(
            "~/input/camera_info", rclcpp::SensorDataQoS(),
            std::bind(&cam_feature::camera_info_callback, this,
                      std::placeholders::_1));

        m_image_sub = create_subscription<sensor_msgs::msg::Image>(
            "~/input/image_raw", rclcpp::SensorDataQoS(),
            std::bind(&cam_feature::image_callback, this,
                      std::placeholders::_1));
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Fail to create topics. Exception: %s",
                     e.what());
        on_deactivate(state);
        return CallbackReturn::FAILURE;
    }

    RCLCPP_INFO(get_logger(), "Activated with %zu plugins.", m_plugins.size());
    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_deactivate(
    const rclcpp_lifecycle::State& /* state */) {
    m_image_sub.reset();
    m_camera_info_sub.reset();
    m_markers_pub.reset();
    m_image_debug_pub.reset();

    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        it->second->deactivate();
    }

    RCLCPP_INFO(get_logger(), "Deactivate.");
    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_cleanup(
    const rclcpp_lifecycle::State& /* state */) {
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        it->second->cleanup();
    }
    m_plugins.clear();

    m_map_client.reset();

    RCLCPP_INFO(get_logger(), "Cleaned up.");
    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_shutdown(
    const rclcpp_lifecycle::State& /* state */) {
    RCLCPP_INFO(get_logger(), "Shutting down");
    return CallbackReturn::SUCCESS;
}

void cam_feature::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr msg) {
    cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg);
    const cv::Mat& image = cv_ptr->image;

    std::shared_ptr<cv::Mat> debug;
    if (m_image_debug_pub->get_subscription_count() != 0) {
        debug = std::make_shared<cv::Mat>(image.clone());
    }

    clover2_pose_msgs::msg::MarkerArray markers;
    cv::Matx33d km;
    cv::Mat_<double> distortion;

    {
        std::lock_guard<std::mutex> guard(m_camera_info_mtx);

        if (!m_camera_model.initialized()) {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
                                 "Camera info not initialized");
            return;
        }

        km = m_camera_model.fullIntrinsicMatrix();
        distortion = m_camera_model.distortionCoeffs();

        markers.header.frame_id = m_camera_model.tfFrame();
        markers.header.stamp = msg->header.stamp;
    }

    std::list<clover2_pose_msgs::msg::Marker> marker_list;
    for (const auto& [name, plugin] : m_plugins) {
        auto poses = plugin->process(msg->header, image, km, distortion, debug);
        marker_list.insert(marker_list.end(), poses.begin(), poses.end());
    }

    markers.markers.reserve(marker_list.size());
    markers.markers.insert(markers.markers.begin(), marker_list.begin(),
                           marker_list.end());

    m_markers_pub->publish(markers);

    {
        std::lock_guard<std::mutex> guard(m_camera_info_mtx);
        ++m_processed_frames;
    }

    if (debug && m_image_debug_pub->get_subscription_count() != 0) {
        cv_bridge::CvImage cv_out;
        cv_out.header.frame_id = msg->header.frame_id;
        cv_out.header.stamp = msg->header.stamp;
        cv_out.encoding = sensor_msgs::image_encodings::BGR8;
        cv_out.image = *debug;
        m_image_debug_pub->publish(*cv_out.toImageMsg());
    }
}

void cam_feature::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    if (msg->height == 0 || msg->width == 0 || msg->d.empty()) {
        return;
    }

    m_camera_model.fromCameraInfo(*msg);
    m_last_camera_info_stamp =
        rclcpp::Time(msg->header.stamp, get_clock()->get_clock_type());
    m_last_camera_info_width = msg->width;
    m_last_camera_info_height = msg->height;
}

void cam_feature::produce_camera_info_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    if (m_camera_model.initialized()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Camera info received");
        stat.add("Camera frame", m_camera_model.tfFrame());
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for Camera Info");
        stat.add("Camera frame", "unknown");
    }

    stat.add("Image width", std::to_string(m_last_camera_info_width));
    stat.add("Image height", std::to_string(m_last_camera_info_height));

    if (m_last_camera_info_stamp.nanoseconds() == 0) {
        stat.add("Camera info age, sec", "never");
    } else {
        stat.add("Camera info age, sec",
                 (now() - m_last_camera_info_stamp).seconds());
    }
}

void cam_feature::produce_map_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    const bool map_valid = m_map_client && m_map_client->valid();

    stat.summary(map_valid ? diagnostic_msgs::msg::DiagnosticStatus::OK
                           : diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                 map_valid ? "Map valid" : "Map invalid or missing");

    stat.add("Map name", map_valid ? m_map_client->get_name() : "unknown");
    stat.add("Map frame", map_valid ? m_map_client->get_map_id() : "unknown");
    stat.add("Marker count",
             map_valid ? std::to_string(m_map_client->get_count()) : "0");
}

void cam_feature::produce_marker_hz_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    const auto current_time = now();

    if (m_last_marker_hz_stamp.nanoseconds() == 0) {
        m_last_marker_hz_stamp = current_time;
        m_last_marker_processed_frames = m_processed_frames;

        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for marker processing samples");
        stat.add("Actual frequency, Hz", m_marker_hz);
        stat.add("Minimum frequency, Hz", m_min_marker_hz);
        stat.add("Maximum frequency, Hz", m_max_marker_hz);
        return;
    }

    const auto dt = (current_time - m_last_marker_hz_stamp).seconds();
    const auto frame_delta =
        m_processed_frames - m_last_marker_processed_frames;

    if (dt > 0.0) {
        m_marker_hz = static_cast<double>(frame_delta) / dt;
    }

    m_last_marker_hz_stamp = current_time;
    m_last_marker_processed_frames = m_processed_frames;

    if (frame_delta == 0) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Marker processing stopped");
    } else if (m_marker_hz >= m_min_marker_hz &&
               m_marker_hz <= m_max_marker_hz) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Marker processing frequency OK");
    } else if (m_marker_hz > m_max_marker_hz) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Marker processing is too fast");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Marker processing is slow");
    }

    stat.add("Actual frequency, Hz", m_marker_hz);
    stat.add("Minimum frequency, Hz", m_min_marker_hz);
    stat.add("Maximum frequency, Hz", m_max_marker_hz);
}

}  // namespace clover2::cam_feature

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::cam_feature::cam_feature)
