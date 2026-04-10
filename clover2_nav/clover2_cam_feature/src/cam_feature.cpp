// clover2
#include <clover2/cam_feature/cam_feature.hpp>

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
#include <stdexcept>

namespace {
constexpr const char* cam_feature_diagnostic_name = "Cam feature status";
}

namespace clover2::cam_feature {

cam_feature::cam_feature(const rclcpp::NodeOptions& options)
    : clover2::common::lifecycle_node("cam_feature", options)
    , m_plugin_loader("clover2_cam_feature",
                      "clover2::cam_feature::plugin_factory")
    , m_executor(rclcpp::ExecutorOptions()) {
    m_diagnostic_updater = get_diagnostic_updater();

    declare_and_watch_parameter<std::vector<std::string>>(
        "plugins", {"aruco"}, [this](const rclcpp::Parameter& p) {
            auto new_list = p.as_string_array();

            for (const auto& type : new_list) {
                if (!m_plugin_loader.isClassAvailable(type)) {
                    throw std::runtime_error("Unknown plugin `" + type + "`");
                }
            }

            m_plugin_types = new_list;
        });

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
    const rclcpp_lifecycle::State& /* state */) {
    m_diagnostic_updater->add(cam_feature_diagnostic_name, this,
                              &cam_feature::produce_diagnostics);

    m_map_client = std::make_shared<clover2::map::client>(shared_from_this());
    load_plugins();

    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_activate(
    const rclcpp_lifecycle::State& /* state */) {
    m_markers_pub = create_publisher<clover2_pose_msgs::msg::MarkerArray>(
        "~/markers", rclcpp::SensorDataQoS());

    m_image_debug_pub = create_publisher<sensor_msgs::msg::Image>(
        "~/debug", rclcpp::SystemDefaultsQoS());

    m_camera_info_sub = create_subscription<sensor_msgs::msg::CameraInfo>(
        "~/camera_info", rclcpp::SensorDataQoS(),
        std::bind(&cam_feature::camera_info_callback, this,
                  std::placeholders::_1));

    m_image_sub = create_subscription<sensor_msgs::msg::Image>(
        "~/image_raw", rclcpp::SensorDataQoS(),
        std::bind(&cam_feature::image_callback, this, std::placeholders::_1));

    RCLCPP_INFO(get_logger(), "Activated with %zu plugins.", m_plugins.size());
    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_deactivate(
    const rclcpp_lifecycle::State& /* state */) {
    m_image_sub.reset();
    m_camera_info_sub.reset();
    m_markers_pub.reset();
    m_image_debug_pub.reset();

    m_map_client.reset();

    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_cleanup(
    const rclcpp_lifecycle::State& /* state */) {
    m_diagnostic_updater->removeByName(cam_feature_diagnostic_name);

    unload_plugins();

    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_shutdown(
    const rclcpp_lifecycle::State& /* state */) {
    m_image_sub.reset();
    m_camera_info_sub.reset();
    m_markers_pub.reset();
    m_image_debug_pub.reset();
    m_map_client.reset();

    unload_plugins();

    return CallbackReturn::SUCCESS;
}

void cam_feature::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    if (!m_camera_model.initialized()) {
        RCLCPP_ERROR(get_logger(), "Camera info not initialized");
        return;
    }

    cv::Mat image = cv_bridge::toCvShare(msg, "bgr8")->image;

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
    for (const auto& plugin : m_plugins) {
        auto poses = plugin->process(image, km, distortion, debug);
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
        stat.add("Last frame pose count", std::to_string(m_last_pose_count));
    }
}

base_plugin::SharedPtr cam_feature::create_plugin(const std::string& type,
                                                  plugin_context& ctx) {
    auto plugin_factory = m_plugin_loader.createSharedInstance(type);

    return plugin_factory->create_plugin_instance(ctx);
}

void cam_feature::add_plugin(const std::string& type, plugin_context& ctx) {
    try {
        base_plugin::SharedPtr plugin = create_plugin(type, ctx);

        RCLCPP_INFO(get_logger(), "Plugin `%s` created", type.c_str());

        m_executor.add_node(plugin->get_node());

        m_plugins.push_back(std::move(plugin));
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Failed to load plugin %s: %s", type.c_str(),
                     e.what());
    }
}

void cam_feature::load_plugins() {
    m_plugins.clear();
    m_plugins.reserve(m_plugin_types.size());

    plugin_context ctx(*this);
    ctx.map_client = m_map_client;

    for (const auto& type : m_plugin_types) {
        add_plugin(type, ctx);
    }
}

void cam_feature::unload_plugins() {
    for (auto plugin : m_plugins) {
        m_executor.remove_node(plugin->get_node());
    }

    m_plugins.clear();
}

}  // namespace clover2::cam_feature

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::cam_feature::cam_feature)
