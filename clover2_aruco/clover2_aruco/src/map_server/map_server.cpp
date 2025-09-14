#include <clover2_aruco/map_server.hpp>

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

}  // namespace clover2_aruco
