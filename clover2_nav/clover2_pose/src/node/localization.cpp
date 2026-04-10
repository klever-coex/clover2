#include "clover2/common/parameter_watcher.hpp"
#include <clover2/pose/localization.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

#include <chrono>
#include <memory>

namespace clover2::pose {

localization::pose(const rclcpp::NodeOptions& options)
    : clover2::common::lifecycle_node("localization", options) {
    m_parameter_watcher =
        std::make_shared<clover2::common::parameter_watcher>(*this);

    m_parameter_watcher->declare_and_watch_parameter<std::string>(
        "sensor", "camera_sensor",
        [this](const rclcpp::Parameter& p) { m_sensor_type = p.as_string(); },
        "Sensor plugin type");

    m_parameter_watcher->declare_and_watch_parameter<std::string>(
        "handler", "aruco",
        [this](const rclcpp::Parameter& p) { m_handler_type = p.as_string(); },
        "Handler type: aruco");

    m_parameter_watcher->declare_and_watch_parameter<double>(
        "optimization_frequency", 30.0,
        [this](const rclcpp::Parameter& p) { m_opt_frequency = p.as_double(); },
        "Graph optimization frequency (Hz)");

    m_parameter_watcher->declare_and_watch_parameter<int>(
        "optimization_iterations", 10,
        [this](const rclcpp::Parameter& p) { m_opt_iterations = p.as_int(); },
        "Optimization iterations per run");

    declare_parameter<std::string>("image_topic", "~/image_raw");
    declare_parameter<std::string>("camera_info_topic", "~/camera_info");
    declare_parameter<std::string>("aruco.dictionary", "DICT_4X4_50");
    declare_parameter<double>("aruco.marker_size", 0.3);

    register_on_configure(
        std::bind(&localization::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&localization::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&localization::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&localization::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&localization::on_shutdown, this, std::placeholders::_1));
}

localization::~localization() {
    m_running = false;
    if (m_graph_thread.joinable()) {
        m_graph_thread.join();
    }
}

localization::CallbackReturn localization::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_frame_queue = std::make_shared<queue::frame_queue>();
    m_graph_manager = std::make_shared<graph::graph_manager>();

    if (m_handler_type == "aruco") {
        handler::aruco_handler::config cfg;
        cfg.dictionary = get_parameter("aruco.dictionary").as_string();
        cfg.default_marker_size =
            get_parameter("aruco.marker_size").as_double();

        cfg.marker_poses[0] = Eigen::Isometry3d::Identity();
        cfg.marker_poses[0].translation() = Eigen::Vector3d(0, 0, 0.001);
        cfg.marker_poses[1] = Eigen::Isometry3d::Identity();
        cfg.marker_poses[1].translation() = Eigen::Vector3d(1, 0, 0.001);
        cfg.marker_poses[2] = Eigen::Isometry3d::Identity();
        cfg.marker_poses[2].translation() = Eigen::Vector3d(1, 1, 0.001);

        m_handler = std::make_unique<handler::aruco_handler>(cfg);
    } else {
        RCLCPP_ERROR(get_logger(), "Unknown handler: %s",
                     m_handler_type.c_str());
        return CallbackReturn::FAILURE;
    }

    return CallbackReturn::SUCCESS;
}

localization::CallbackReturn localization::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_pose_pub = create_publisher<geometry_msgs::msg::PoseStamped>(
        "~/pose", rclcpp::SystemDefaultsQoS());

    try {
        pluginlib::ClassLoader<sensor::base_sensor> loader(
            "clover2_pose",
            "clover2::pose::sensor::base_sensor");
        m_sensor = loader.createSharedInstance(m_sensor_type);

        sensor::sensor_params params;
        params.sensor_id = 0;
        params.image_topic = get_parameter("image_topic").as_string();
        params.camera_info_topic =
            get_parameter("camera_info_topic").as_string();

        m_sensor_node = std::make_shared<rclcpp::Node>("localization_sensor");
        m_sensor->init(m_sensor_node,
                       std::bind(&localization::sensor_callback, this,
                                 std::placeholders::_1),
                       params);
        m_sensor->start();
    } catch (const pluginlib::PluginlibException& e) {
        RCLCPP_ERROR(get_logger(), "Failed to load sensor plugin: %s",
                     e.what());
        return CallbackReturn::FAILURE;
    }

    m_running = true;
    m_graph_thread = std::thread(&localization::graph_thread_func, this);

    return CallbackReturn::SUCCESS;
}

localization::CallbackReturn localization::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_running = false;
    if (m_sensor) {
        m_sensor->stop();
    }
    if (m_graph_thread.joinable()) {
        m_graph_thread.join();
    }
    m_pose_pub.reset();
    m_sensor.reset();
    m_sensor_node.reset();

    return CallbackReturn::SUCCESS;
}

rclcpp::node_interfaces::NodeBaseInterface::SharedPtr
localization::get_sensor_node_base_interface() const {
    return m_sensor_node ? m_sensor_node->get_node_base_interface() : nullptr;
}

localization::CallbackReturn localization::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_frame_queue.reset();
    m_graph_manager.reset();
    m_handler.reset();
    return CallbackReturn::SUCCESS;
}

localization::CallbackReturn localization::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    return CallbackReturn::SUCCESS;
}

void localization::sensor_callback(const data::sensor_data& data) {
    if (!m_handler) {
        return;
    }
    auto frame = m_handler->process(data);
    if (frame.observations.empty()) {
        return;
    }
    if (m_frame_queue) {
        m_frame_queue->push(std::move(frame));
    }
}

void localization::graph_thread_func() {
    auto period = std::chrono::duration<double>(1.0 / m_opt_frequency);
    auto last_opt = std::chrono::steady_clock::now();

    while (m_running) {
        if (m_frame_queue && m_graph_manager) {
            while (auto frame = m_frame_queue->try_pop()) {
                m_graph_manager->add_frame(*frame);
            }

            auto now = std::chrono::steady_clock::now();
            if (now - last_opt >= period) {
                m_graph_manager->optimize(m_opt_iterations);
                last_opt = now;

                auto pose = m_graph_manager->get_current_pose();
                if (m_pose_pub && m_pose_pub->get_subscription_count() > 0) {
                    geometry_msgs::msg::PoseStamped msg;
                    msg.header.stamp = this->get_clock()->now();
                    msg.header.frame_id = "map";
                    msg.pose = tf2::toMsg(pose);
                    m_pose_pub->publish(msg);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

}  // namespace clover2::pose
