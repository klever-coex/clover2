#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// Msgs includes
#include <clover2_aruco_msgs/msg/marker_map.hpp>

// Srvs includes
#include <clover2_aruco_msgs/srv/get_map.hpp>

namespace clover2_aruco {

/**
 * @class map_server
 * @brief ROS2 node that provides a service to retrieve ArUco marker maps.
 * 
 * This node loads a marker map from a file and exposes it via the
 * clover2_aruco_msgs::srv::GetMap service. Supports both legacy and YAML map formats.
 */
class map_server : public rclcpp::Node {
public:
    using SharedPtr = std::shared_ptr<map_server>; ///< Shared pointer type for map_server
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult; ///< Type alias for parameter callbacks

    /**
     * @brief Construct a new map_server node.
     * @param options Node options for ROS2 node configuration.
     */
    explicit map_server(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    /**
     * @brief Callback for the GetMap service.
     * @param request Service request containing parameters (currently unused).
     * @param response Service response to be filled with the marker map.
     */
    void map_callback(
        const clover2_aruco_msgs::srv::GetMap::Request::SharedPtr request,
        clover2_aruco_msgs::srv::GetMap::Response::SharedPtr response);

    /**
     * @brief Parse a legacy map file format into a MarkerMap message.
     * @param filename Path to the legacy map file.
     * @return Shared pointer to the parsed MarkerMap message.
     */
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_legacy(
        const std::string& filename) const;

    /**
     * @brief Parse a YAML map file into a MarkerMap message.
     * @param filename Path to the YAML map file.
     * @return Shared pointer to the parsed MarkerMap message.
     */
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_yaml(
        const std::string& filename) const;

    /**
     * @brief Update the internal map stored in the node.
     * @param map Shared pointer to the new MarkerMap message.
     */
    void update_map(clover2_aruco_msgs::msg::MarkerMap::SharedPtr map);

    /**
     * @brief Append a marker to an existing MarkerMap.
     * @param map MarkerMap to append the marker to.
     * @param id Marker ID.
     * @param length Length of the marker (meters).
     * @param x X position of the marker.
     * @param y Y position of the marker.
     * @param z Z position of the marker.
     * @param roll Roll rotation of the marker (radians).
     * @param pitch Pitch rotation of the marker (radians).
     * @param yaw Yaw rotation of the marker (radians).
     */
    void map_append_marker(clover2_aruco_msgs::msg::MarkerMap::SharedPtr& map,
                           int id, double length, double x, double y, double z,
                           double roll, double pitch, double yaw);

    std::string m_map_path; ///< Path to the map file
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr m_map_msg; ///< Current MarkerMap message

    rclcpp::Node::OnSetParametersCallbackHandle::SharedPtr
        m_set_parameters_handle_ptr; ///< Handle for ROS2 parameter callbacks

    rclcpp::Server<clover2_aruco_msgs::srv::GetMap>::SharedPtr m_map_server; ///< ROS2 service server
};

}  // namespace clover2_aruco
