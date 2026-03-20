#pragma once

// clover2
#include <clover2/common/lifecycle_node.hpp>
#include <clover2/common/parameter_watcher.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <tf2_ros/static_transform_broadcaster.hpp>
#include <tf2_ros/transform_broadcaster.hpp>

// msgs
#include <clover2_aruco_msgs/msg/marker_map.hpp>
#include <std_msgs/msg/empty.hpp>

// srvs
#include <clover2_aruco_msgs/srv/get_map.hpp>

// STL
#include <filesystem>
#include <memory>
#include <mutex>

namespace clover2::aruco {

/**
 * @class map_server
 * @brief ROS2 node that provides a service to retrieve ArUco marker maps.
 *
 * This node loads a marker map from a file and exposes it via the
 * clover2_aruco_msgs::srv::GetMap service. Supports both legacy and YAML map
 * formats.
 */
class map_server : public clover2::common::lifecycle_node {
public:
    using SharedPtr =
        std::shared_ptr<map_server>;  ///< Shared pointer type for map_server
    using SetParametersResult =
        rcl_interfaces::msg::SetParametersResult;  ///< Type alias for parameter
                                                   ///< callbacks

    /**
     * @brief Construct a new map_server node.
     * @param options Node options for ROS2 node configuration.
     */
    explicit map_server(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    /**
     * @brief Lifecycle callback: configure the node.
     */
    CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: activate the node.
     */
    CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: deactivate the node.
     */
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: cleanup resources.
     */
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: shutdown the node.
     */
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

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
     * @brief Parse a map file into a MarkerMap message.
     * @param filename Path to map file.
     * @return Shared pointer to the parsed MarkerMap message.
     */
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_map(
        const std::filesystem::path& filename) const;

    /**
     * @brief Send update trigger
     * @param filename Path to map file.
     */
    void update_map(const std::filesystem::path& filename);

    /**
     * @brief Send update trigger
     * @param filename Path to map file.
     */
    void update_map(clover2_aruco_msgs::msg::MarkerMap::SharedPtr new_map);

    std::recursive_mutex m_map_mtx;    ///< Map update mutex
    std::filesystem::path m_map_path;  ///< Path to the map file
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr
        m_map_msg;  ///< Current MarkerMap message

    // TF
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster>
        m_tf_static_broadcaster;  ///< TF static broadcaster

    rclcpp::TimerBase::SharedPtr m_map_notify_timer;

    // Parameter watcher
    clover2::common::parameter_watcher::SharedPtr m_parameter_watcher;

    rclcpp::Service<clover2_aruco_msgs::srv::GetMap>::SharedPtr
        m_map_server;  ///< ROS2 service server
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr
        m_map_update_pub;  ///< ROS2 trigger publisher
};

}  // namespace clover2::aruco
