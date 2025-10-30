#include <clover2_aruco/map_server.hpp>
#include <clover2_aruco/utils/map_io.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_msgs/msg/tf_message.hpp>

#include <fstream>

namespace clover2_aruco {

map_server::map_server(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("map_server", options)
    , m_map_path("") {
    enable_watch_parameters();

    declare_and_watch_parameter<std::string>(
        "map", "",
        [this](const rclcpp::Parameter& p) { m_map_path = p.as_string(); },
        "Path to map file whit .txt/.yaml/.yml extension.");

    declare_and_watch_parameter<bool>(
        "tf_publish", true,
        [this](const rclcpp::Parameter& p) { m_tf_publish = p.as_bool(); },
        "Enable map markers transform pub.");

    register_on_configure(
        std::bind(&map_server::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&map_server::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&map_server::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&map_server::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&map_server::on_shutdown, this, std::placeholders::_1));
}

map_server::CallbackReturn map_server::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    try {
        RCLCPP_DEBUG(get_logger(), "Loading map: '%s'", m_map_path.c_str());
        m_map_msg = parse_map(m_map_path);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Configure error: %s", e.what());
        return map_server::CallbackReturn::ERROR;
    }

    return map_server::CallbackReturn::SUCCESS;
}

map_server::CallbackReturn map_server::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    if (!m_map_msg) {
        RCLCPP_ERROR(get_logger(), "Map dont set");
        return map_server::CallbackReturn::ERROR;
    }

    m_map_update_pub = this->create_publisher<std_msgs::msg::Empty>(
        "~/map_update", rclcpp::SensorDataQoS());
    m_tf_static_broadcaster =
        std::make_shared<tf2_ros::StaticTransformBroadcaster>(*this);

    m_map_server = this->create_service<clover2_aruco_msgs::srv::GetMap>(
        "~/get_map", std::bind(&map_server::map_callback, this,
                               std::placeholders::_1, std::placeholders::_2));

    m_map_notify_timer =
        this->create_wall_timer(std::chrono::seconds(0), [this]() {
            try {
                RCLCPP_DEBUG(get_logger(), "Using map '%s'",
                             m_map_msg->name.c_str());
                update_map(m_map_path);
            } catch (const std::exception& e) {
                RCLCPP_ERROR(get_logger(), "Configure error: %s", e.what());
            }

            m_map_notify_timer.reset();
        });

    return map_server::CallbackReturn::SUCCESS;
}

map_server::CallbackReturn map_server::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_static_broadcaster.reset();
    m_map_server.reset();
    m_map_update_pub.reset();

    return map_server::CallbackReturn::SUCCESS;
}

map_server::CallbackReturn map_server::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_map_msg.reset();

    return map_server::CallbackReturn::SUCCESS;
}

map_server::CallbackReturn map_server::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_static_broadcaster.reset();
    m_map_server.reset();
    m_map_update_pub.reset();

    return map_server::CallbackReturn::SUCCESS;
}

void map_server::map_callback(
    const clover2_aruco_msgs::srv::GetMap::Request::SharedPtr /* request */,
    clover2_aruco_msgs::srv::GetMap::Response::SharedPtr response) {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    response->map = *m_map_msg;
}

clover2_aruco_msgs::msg::MarkerMap::SharedPtr map_server::parse_map(
    const std::filesystem::path& filename) const {
    std::string extension = filename.extension().string();

    auto map = std::make_shared<clover2_aruco_msgs::msg::MarkerMap>();

    if (extension == ".txt") {
        RCLCPP_DEBUG(get_logger(), "Detect legacy map format");
        clover2_aruco::utils::load_from_txt(filename, *map);
    } else if (extension == ".yaml" || extension == ".yml") {
        RCLCPP_DEBUG(get_logger(), "Detect new map format");
        clover2_aruco::utils::load_from_yaml(filename, *map);
    } else {
        throw std::runtime_error("Unexpected map format: " + extension);
    }

    return map;
}

void map_server::update_map(const std::filesystem::path& filename) {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    auto new_map = parse_map(filename);
    update_map(new_map);
}

void map_server::update_map(
    const clover2_aruco_msgs::msg::MarkerMap::SharedPtr new_map) {
    std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

    m_map_msg = new_map;

    if (m_tf_publish) {
        std::vector<geometry_msgs::msg::TransformStamped> transforms;
        transforms.reserve(m_map_msg->markers.size());

        for (const auto& it : m_map_msg->markers) {
            geometry_msgs::msg::TransformStamped transform;

            transform.header.frame_id = m_map_msg->header.frame_id;
            transform.header.stamp = get_clock()->now();

            transform.child_frame_id =
                m_map_msg->header.frame_id + "_aruco_" + std::to_string(it.id);

            tf2::Transform t;
            tf2::fromMsg(it.pose, t);
            tf2::toMsg(t, transform.transform);

            transforms.push_back(transform);
        }

        m_tf_static_broadcaster->sendTransform(transforms);
    }

    m_map_update_pub->publish(std_msgs::msg::Empty());
}

}  // namespace clover2_aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_aruco::map_server)
