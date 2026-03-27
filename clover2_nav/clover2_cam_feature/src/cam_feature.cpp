#include <clover2_cam_feature/cam_feature.hpp>

#include <cv_bridge/cv_bridge.hpp>

#include <functional>

namespace clover2_cam_feature {

cam_feature::cam_feature(const rclcpp::NodeOptions& options)
    : clover2::common::lifecycle_node("cam_feature", options),
      m_last_pose_count(0) {
    m_parameter_watcher =
        std::make_shared<clover2::common::parameter_watcher>(*this);

    enable_diagnostic_updater();
    m_diagnostic_updater = get_diagnostic_updater();

    declare_parameter<std::vector<std::string>>(
        "plugins", std::vector<std::string>{"aruco"});
    declare_parameter<std::vector<std::string>>(
        "plugin_types",
        std::vector<std::string>{"clover2_cam_feature::plugins::aruco"});

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
    m_plugin_ids = get_parameter("plugins").as_string_array();
    m_plugin_types = get_parameter("plugin_types").as_string_array();

    if (m_plugin_ids.size() != m_plugin_types.size()) {
        RCLCPP_ERROR(
            get_logger(),
            "plugins and plugin_types must have the same length "
            "(Nav2-style parallel arrays)");
        return CallbackReturn::FAILURE;
    }

    if (m_plugin_ids.empty()) {
        RCLCPP_ERROR(get_logger(), "plugins list is empty");
        return CallbackReturn::FAILURE;
    }

    m_diagnostic_updater->add("Cam feature status", this,
                              &cam_feature::produce_diagnostics);

    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::load_plugins() {
    m_plugin_loader = std::make_unique<pluginlib::ClassLoader<base_plugin>>(
        "clover2_cam_feature", "clover2_cam_feature::base_plugin");

    m_plugins.clear();
    m_plugins.reserve(m_plugin_ids.size());

    for (size_t i = 0; i < m_plugin_ids.size(); ++i) {
        try {
            base_plugin::SharedPtr plugin =
                m_plugin_loader->createSharedInstance(m_plugin_types[i]);
            plugin_context ctx(*this);
            ctx.map_client = m_map_client;
            plugin->init_plugin(m_plugin_ids[i], ctx);
            m_plugins.push_back(std::move(plugin));
        } catch (const pluginlib::PluginlibException& e) {
            RCLCPP_ERROR(get_logger(), "Failed to load plugin %s: %s",
                         m_plugin_types[i].c_str(), e.what());
            m_plugins.clear();
            m_plugin_loader.reset();
            return CallbackReturn::FAILURE;
        }
    }

    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_activate(
    const rclcpp_lifecycle::State& /* state */) {
    m_map_client = std::make_shared<clover2::aruco::map_client>(
        shared_from_this());

    const auto load_rc = load_plugins();
    if (load_rc != CallbackReturn::SUCCESS) {
        return load_rc;
    }

    m_poses_pub =
        create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>(
            "~/poses", rclcpp::SensorDataQoS());

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
    m_poses_pub.reset();

    m_plugins.clear();
    m_plugin_loader.reset();
    m_map_client.reset();

    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_cleanup(
    const rclcpp_lifecycle::State& /* state */) {
    m_diagnostic_updater->removeByName("Cam feature status");
    m_plugin_ids.clear();
    m_plugin_types.clear();
    return CallbackReturn::SUCCESS;
}

cam_feature::CallbackReturn cam_feature::on_shutdown(
    const rclcpp_lifecycle::State& /* state */) {
    m_image_sub.reset();
    m_camera_info_sub.reset();
    m_poses_pub.reset();
    m_plugins.clear();
    m_plugin_loader.reset();
    m_map_client.reset();
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

    const cv::Mat& km = m_camera_model.fullIntrinsicMatrix();
    cv::Matx33d K;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            K(r, c) = km.at<double>(r, c);
        }
    }

    cv::Mat_<double> distortion;
    m_camera_model.distortionCoeffs().convertTo(distortion, CV_64F);

    size_t pose_count = 0;
    for (const auto& plugin : m_plugins) {
        auto poses =
            plugin->process(image, K, distortion, nullptr);
        for (auto& pose : poses) {
            pose.header.frame_id = m_camera_model.tfFrame();
            pose.header.stamp = msg->header.stamp;
            m_poses_pub->publish(pose);
            ++pose_count;
        }
    }
    m_last_pose_count = pose_count;
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

}  // namespace clover2_cam_feature

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(clover2_cam_feature::cam_feature)
