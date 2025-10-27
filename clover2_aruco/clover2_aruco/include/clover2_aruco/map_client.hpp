#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

// Msgs includes
#include <clover2_aruco_msgs/msg/marker_map.hpp>
#include <std_msgs/msg/empty.hpp>

// Srvs includes
#include <clover2_aruco_msgs/srv/get_map.hpp>

// STL includes
#include <unordered_map>
#include <vector>

namespace clover2_aruco {

/**
 * @class map_client
 * @brief ROS2 client to retrieve and manage ArUco marker maps from a map
 * server.
 *
 * This class subscribes to map update notifications and provides convenient
 * access to marker map information, including marker IDs, sizes, and map
 * metadata.
 */
class map_client {
public:

    struct marker {
        explicit marker(int id, double size)
        : id(id)
        , size(size) {
        }

        void set_transform(const geometry_msgs::msg::Pose& pose) {
            tf2::fromMsg(pose, transform);
        }

        int id;
        double size;
        tf2::Transform transform;
    };
    
    /**
     * @brief Construct a new map_client object.
     *
     * The constructor sets up a subscription to map updates and initializes a
     * client for the GetMap service.
     *
     * @tparam NodeT Type of ROS2 node (rclcpp::Node or derived)
     * @param node Shared pointer or reference to the node
     * @param cb_group Optional callback group for service calls
     */
    template <typename NodeT>
    explicit map_client(const NodeT& node,
                        rclcpp::CallbackGroup::SharedPtr cb_group = nullptr)
        : m_logger(node->get_logger().get_child("map_client"))
        , m_map_valid(false)
        , m_name("") {
        m_map_update_sub =
            node->template create_subscription<std_msgs::msg::Empty>(
                "~/map_update", rclcpp::SensorDataQoS(),
                std::bind(&map_client::map_update_callback, this,
                          std::placeholders::_1));

        if (cb_group) {
            m_map_client =
                node->template create_client<clover2_aruco_msgs::srv::GetMap>(
                    "~/get_map", rclcpp::ServicesQoS(), cb_group);
        } else {
            m_map_client =
                node->template create_client<clover2_aruco_msgs::srv::GetMap>(
                    "~/get_map", rclcpp::ServicesQoS());
        }

        update_map();
    }

    /**
     * @brief Check if the current map is valid.
     * @return true if the map has been loaded successfully
     * @return false otherwise
     */
    bool valid() const { return m_map_valid; }

    /**
     * @brief Get the name of the current map.
     * @return const char* Name of the map
     */
    const char* get_name() const { return m_name.c_str(); }

    /**
     * @brief Get the size of a specific marker by ID.
     * @param id Marker ID
     * @return int Size of the marker
     */
    double get_marker_size(int id) const { return m_sizes.at(id); }

    /**
     * @brief Get the number of markers in the current map.
     * @return int Number of markers
     */
    int get_count() const { return m_sizes.size(); }

    bool has_marker(int id) const { return m_sizes.find(id) != m_sizes.end(); };

private:
    /**
     * @brief Callback for map update notifications.
     *
     * This triggers a map update request whenever a notification is received.
     * @param msg Empty message (unused)
     */
    void map_update_callback(const std_msgs::msg::Empty::SharedPtr /* msg */) {
        update_map();
    }

    /**
     * @brief Update internal map data from a MarkerMap message.
     * @param msg MarkerMap message received from the server
     */
    void update_cached_map(const clover2_aruco_msgs::msg::MarkerMap& msg) {
        m_name = msg.name;

        //m_idx.clear();
        m_sizes.clear();

        //m_idx.reserve(msg.markers.size());
        m_sizes.reserve(msg.markers.size());

        for (size_t i = 0; i < msg.markers.size(); i++) {
            //m_idx[i] = msg.markers[i].id;
            m_sizes[msg.markers[i].id] = msg.markers[i].size;
        }

        m_map_valid = true;
    }

    /**
     * @brief Request a map update from the server.
     *
     * Sends a GetMap service request and updates internal data upon response.
     */
    void update_map() {
        auto map_request =
            std::make_shared<clover2_aruco_msgs::srv::GetMap::Request>();
        m_map_client->async_send_request(
            map_request,
            [this](rclcpp::Client<clover2_aruco_msgs::srv::GetMap>::SharedFuture
                       future) {
                if (!future.valid()) {
                    RCLCPP_ERROR(m_logger, "Fail to get map");
                    return;
                }

                auto resp = future.get();
                RCLCPP_INFO(m_logger,
                            "Update map from %s to %s with %ld markers",
                            get_name(), resp->map.name.c_str(),
                            resp->map.markers.size());

                update_cached_map(resp->map);
            });
    }

    rclcpp::Logger m_logger;  ///< Logger for this class
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr
        m_map_update_sub;  ///< Subscription to map update notifications
    rclcpp::Client<clover2_aruco_msgs::srv::GetMap>::SharedPtr
        m_map_client;  ///< Client for GetMap service

    rclcpp::TimerBase::SharedPtr m_map_init_timer;

    bool m_map_valid;        ///< True if the map has been successfully loaded
    std::string m_name;      ///< Name of the current map
    std::unordered_map<int, double>
        m_sizes;  ///< Map from marker ID to marker size
};

}  // namespace clover2_aruco
