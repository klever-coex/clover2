#include <clover2_aruco/map_server.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <yaml-cpp/yaml.h>

#include <fstream>

namespace YAML {
template <>
struct convert<clover2_aruco_msgs::msg::Marker> {
    static bool decode(const Node& node,
                       clover2_aruco_msgs::msg::Marker& marker) {
        if (!node.IsMap()) {
            return false;
        }

        if (!node["id"] || !node["size"]) {
            return false;
        }

        marker.id = node["id"].as<int>();
        marker.size = node["size"].as<double>();

        marker.pose.position.x = 0.0;
        marker.pose.position.y = 0.0;
        marker.pose.position.z = 0.0;

        if (node["pose"]) {
            auto& pose = node["pose"];
            if (pose["x"]) {
                marker.pose.position.x = pose["x"].as<double>();
            }

            if (pose["y"]) {
                marker.pose.position.y = pose["y"].as<double>();
            }

            if (pose["z"]) {
                marker.pose.position.z = pose["z"].as<double>();
            }
        }

        tf2::Quaternion q;
        if (node["rot"]) {
            auto& rot = node["rot"];
            double roll = 0.0, pitch = 0.0, yaw = 0.0;

            if (rot["roll"]) {
                roll = rot["roll"].as<double>();
            }

            if (rot["pitch"]) {
                pitch = rot["pitch"].as<double>();
            }

            if (rot["yaw"]) {
                yaw = rot["yaw"].as<double>();
            }

            q.setRPY(roll, pitch, yaw);
        } else if (node["quat"]) {
            auto& quat = node["quat"];

            if (!quat["x"] || !quat["y"] || !quat["z"] || !quat["w"]) {
                return false;
            }

            q.setValue(quat["x"].as<double>(), quat["y"].as<double>(),
                       quat["z"].as<double>(), quat["w"].as<double>());
        } else {
            q.setRPY(0.0, 0.0, 0.0);
        }

        marker.pose.orientation.x = q.x();
        marker.pose.orientation.y = q.y();
        marker.pose.orientation.z = q.z();
        marker.pose.orientation.w = q.w();

        return true;
    }
};

}  // namespace YAML

namespace clover2_aruco {

map_server::map_server(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("map_server", options)
    , m_map_path("")
    , m_map_msg(std::make_shared<clover2_aruco_msgs::msg::MarkerMap>()) {
    enable_watch_parameters();

    declare_and_watch_parameter<std::string>(
        "map", "",
        [this](const rclcpp::Parameter& p) {
            m_map_path = p.as_string();
        },
        "Path to map file whit .txt/.yaml/.yml extension.");

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

    m_map_update_pub =
        this->create_publisher<std_msgs::msg::Empty>("~/map_update", 1);

    try {
        RCLCPP_DEBUG(get_logger(), "Using map '%s'", m_map_msg->name.c_str());
        update_trigger();
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Configure error: %s", e.what());
        return map_server::CallbackReturn::ERROR;
    }

    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(*this);
    m_map_server = this->create_service<clover2_aruco_msgs::srv::GetMap>(
        "~/get_map", std::bind(&map_server::map_callback, this,
                               std::placeholders::_1, std::placeholders::_2));

    return map_server::CallbackReturn::SUCCESS;
}

map_server::CallbackReturn map_server::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster.reset();
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
    m_tf_broadcaster.reset();
    m_map_server.reset();
    m_map_update_pub.reset();

    return map_server::CallbackReturn::SUCCESS;
}

void map_server::map_callback(
    const clover2_aruco_msgs::srv::GetMap::Request::SharedPtr /* request */,
    clover2_aruco_msgs::srv::GetMap::Response::SharedPtr response) {
    response->map = *m_map_msg;
}

clover2_aruco_msgs::msg::MarkerMap::SharedPtr map_server::parse_legacy(
    const std::filesystem::path& filename) const {
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr map =
        std::make_shared<clover2_aruco_msgs::msg::MarkerMap>();
    map->name = filename.stem();

    std::ifstream f(filename);
    if (!f.good()) {
        throw std::runtime_error("Unable to open " + filename.string() +
                                 ", reason: " + strerror(errno));
    }

    std::string line;
    while (std::getline(f, line)) {
        int id;
        double length, x, y, z, yaw, pitch, roll;

        std::istringstream s(line);

        char first = 0;
        if (!(s >> first) || first == '#') {
            continue;
        }

        if (std::isdigit(first)) {
            s.putback(first);
        } else {
            throw std::runtime_error("Malformed input: " + line);
        }

        if (!(s >> id >> length >> x >> y)) {
            RCLCPP_ERROR(
                get_logger(),
                "Not enough data in line: %s; "
                "Each marker must have at least id, length, x, y fields",
                line.c_str());
            continue;
        }

        if (!(s >> z)) {
            RCLCPP_DEBUG(get_logger(),
                         "No z coordinate provided for marker %d, assuming 0",
                         id);
            z = 0;
        }

        if (!(s >> yaw)) {
            RCLCPP_DEBUG(get_logger(),
                         "No yaw provided for marker %d, assuming 0", id);
            yaw = 0;
        }

        if (!(s >> pitch)) {
            RCLCPP_DEBUG(get_logger(),
                         "No pitch provided for marker %d, assuming 0", id);
            pitch = 0;
        }

        if (!(s >> roll)) {
            RCLCPP_DEBUG(get_logger(),
                         "No roll provided for marker %d, assuming 0", id);
            roll = 0;
        }

        map_append_marker(map, id, length, x, y, z, yaw, pitch, roll);
    }

    map->map_load_time = get_clock()->now();

    return map;
}

clover2_aruco_msgs::msg::MarkerMap::SharedPtr map_server::parse_yaml(
    const std::filesystem::path& filename) const {
    YAML::Node config = YAML::LoadFile(filename);

    clover2_aruco_msgs::msg::MarkerMap::SharedPtr map =
        std::make_shared<clover2_aruco_msgs::msg::MarkerMap>();

    map->name = config["name"].as<std::string>();
    RCLCPP_DEBUG(get_logger(), "Parsing map '%s'", map->name.c_str());

    if (config["frame_id"]) {
        map->header.frame_id = config["frame_id"].as<std::string>();
    } else {
        map->header.frame_id = "map";
        RCLCPP_WARN(get_logger(), "Map frame_id dont set. Use default 'map'");
    }

    RCLCPP_DEBUG(get_logger(), "frame_id: %s", map->header.frame_id.c_str());

    if (config["markers"].IsSequence()) {
        for (const auto& it : config["markers"]) {
            auto marker = it.as<clover2_aruco_msgs::msg::Marker>();
            map->markers.push_back(marker);
            RCLCPP_DEBUG(get_logger(), "Add marker %d with size %.02f", marker.id, marker.size);
        }
    } else {
        RCLCPP_WARN(get_logger(), "Map is empty.");
    }

    return map;
}

clover2_aruco_msgs::msg::MarkerMap::SharedPtr map_server::parse_map(
    const std::filesystem::path& filename) const {
    std::string extension = filename.extension().string();

    if (extension == ".txt") {
        RCLCPP_DEBUG(get_logger(), "Detect legacy map format");
        return parse_legacy(filename);
    } else if (extension == ".yaml" || extension == ".yml") {
        RCLCPP_DEBUG(get_logger(), "Detect new map format");
        return parse_yaml(filename);
    } else {
        throw std::runtime_error("Unexpected map format: " + extension);
    }

    return std::make_shared<clover2_aruco_msgs::msg::MarkerMap>();
}

void map_server::update_trigger() {
    RCLCPP_INFO(get_logger(), "Send map update trigger.");

    m_map_update_pub->publish(std_msgs::msg::Empty());
}

void map_server::map_append_marker(
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr& map, int id, double size,
    double x, double y, double z, double roll, double pitch, double yaw) const {
    clover2_aruco_msgs::msg::Marker marker;

    marker.id = id;

    marker.size = size;

    marker.pose.position.x = x;
    marker.pose.position.y = y;
    marker.pose.position.z = z;

    tf2::Quaternion q;
    q.setRPY(roll, pitch, yaw);
    tf2::Transform transform(q, tf2::Vector3(x, y, z));
    marker.pose.orientation.x = q.x();
    marker.pose.orientation.y = q.y();
    marker.pose.orientation.z = q.z();
    marker.pose.orientation.w = q.w();
}

}  // namespace clover2_aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_aruco::map_server)
