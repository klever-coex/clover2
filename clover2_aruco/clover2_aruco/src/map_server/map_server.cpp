#include <clover2_aruco/map_server.hpp>

#include <filesystem>

namespace clover2_aruco {

map_server::map_server(const rclcpp::NodeOptions& options)
    : rclcpp::Node("map_server", options)
    , m_map_path("")
    , m_map(std::make_shared<clover2_aruco_msgs::msg::MarkerMap>()) {
    m_map_server = this->create_server<clover2_aruco_msgs::srv::GetMap>(
        "~/get_map", &map_server::map_callback);
}

void map_server::map_callback(
    const clover2_aruco_msgs::srv::GetMap::Request::SharedPtr /* request */,
    clover2_aruco_msgs::srv::GetMap::Response::SharedPtr response) {
    response->map = m_map_msg;
}

clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_legacy(
    const std::string& filename) const {
    std::filesystem::path filepath(filename);

    clover2_aruco_msgs::msg::MarkerMap::SharedPtr map =
        std::make_shared<clover2_aruco_msgs::msg::MarkerMap>();
    map->name = filepath.stem();

    std::ifstream f(filepath);
    if (!f.good()) {
        RCLCPP_ERROR(get_logger(), "%s - %s", strerror(errno),
                     filename.c_str());
        return std::nullptr;
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
            RCLCPP_ERROR(get_logger(), "Malformed input: %s", line.c_str());
            return std::nullptr;
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

        map_append_marker(id, length, x, y, z, yaw, pitch, roll);
    }

    map->map_load_time = get_clock()->now();

    return map;
}

clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_yaml(
    const std::string& filename) const {}

void map_append_marker(clover2_aruco_msgs::msg::MarkerMap::SharedPtr& map,
                       int id, double length, double x, double y, double z,
                       double roll, double pitch, double yaw) {
    clover2_aruco_msgs::msg::Marker marker;

    marker.id = id;

    marker.length = length;

    marker.pose.position.x = x;
    marker.pose.position.y = y;
    marker.pose.position.z = z;

    marker.pose.orientation.x =

        map->markers.assign();
}

}  // namespace clover2_aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_aruco::map_server)
