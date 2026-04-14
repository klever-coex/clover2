// clover2
#include <clover2/cam_feature/cam_feature.hpp>
#include <clover2/common/lifecycle_node.hpp>
#include <clover2/common/util/parameter.hpp>

// opencv
#include <cv_bridge/cv_bridge.hpp>

// ROS2
#include <rclcpp/executor.hpp>
#include <rclcpp/logging.hpp>
#include <sensor_msgs/image_encodings.hpp>

// STL
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr const char* cam_feature_diagnostic_name = "Cam feature status";
const std::vector<std::string> default_plugin_ids = {"aruco"};
const std::vector<std::string> default_plugin_types = {
    "clover2::cam_feature::plugins::aruco"};

}  // namespace

namespace clover2::cam_feature {

cam_feature::cam_feature(const rclcpp::NodeOptions& options)
    : clover2::common::lifecycle_node("cam_feature", options) {
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
    auto node = shared_from_this();

    get_diagnostic_updater()->add(cam_feature_diagnostic_name, this,
                                  &cam_feature::produce_diagnostics);

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
            clover2::common::util::declare_parameter_if_not_declared(
                node, param_name, default_plugin_types[i]);
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

            plugin->configure(id, node, m_map_client);

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
    get_diagnostic_updater()->removeByName(cam_feature_diagnostic_name);

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
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    if (!m_camera_model.initialized()) {
        RCLCPP_ERROR(get_logger(), "Camera info not initialized");
        return;
    }

    cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg);
    const cv::Mat& image = cv_ptr->image;

    const cv::Matx33d& km = m_camera_model.fullIntrinsicMatrix();
    cv::Mat_<double> distortion = m_camera_model.distortionCoeffs();

    std::shared_ptr<cv::Mat> debug;
    if (m_image_debug_pub->get_subscription_count() != 0) {
        debug = std::make_shared<cv::Mat>(image.clone());
    }

    clover2_pose_msgs::msg::MarkerArray markers;
    markers.header.frame_id = m_camera_model.tfFrame();
    markers.header.stamp = msg->header.stamp;

    std::list<clover2_pose_msgs::msg::Marker> marker_list;
    for (const auto& [name, plugin] : m_plugins) {
        auto poses = plugin->process(msg->header, image, km, distortion, debug);
        markers.markers.reserve(markers.markers.size() + poses.size());
        marker_list.insert(marker_list.end(), poses.begin(), poses.end());
    }

    m_last_pose_count = marker_list.size();

    markers.markers.reserve(m_last_pose_count);
    markers.markers.insert(markers.markers.begin(), marker_list.begin(),
                           marker_list.end());

    m_markers_pub->publish(markers);

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
}

void cam_feature::produce_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_camera_model.initialized()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for Camera Info");
    } else if (!m_map_client || !m_map_client->valid()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Map invalid or missing");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "Running");
        stat.add("Camera frame", m_camera_model.tfFrame());
        stat.add("Plugins loaded", std::to_string(m_plugins.size()));
    }
}

}  // namespace clover2::cam_feature

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::cam_feature::cam_feature)
