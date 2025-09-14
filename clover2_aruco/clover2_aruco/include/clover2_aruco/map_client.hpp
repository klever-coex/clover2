#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>

// Msgs includes
#include <clover2_aruco_msgs/msg/marker_map.hpp>
#include <std_msgs/msg/empty.hpp>

// Srvs includes
#include <clover2_aruco_msgs/srv/get_map.hpp>

// STL includes
#include <unordered_map>
#include <vector>

namespace clover2_aruco {

class map_client {
public:
    template <typename NodeT>
    explicit map_client(const NodeT& node,
                        rclcpp::CallbackGroup::SharedPtr cb_group = nullptr)
        : m_logger(node->get_logger().get_child("map_client"))
        , m_map_valid(false)
        , m_name("")
        , m_idx(0) {
        m_map_update_sub =
            node->template create_subscription<std_msgs::msg::Empty>(
                "~/map_update", 1,
                std::bind(&map_client::map_update_callback, this,
                          std::placeholders::_1));

        if (cb_group) {
            m_map_client =
                node->template create_client<clover2_aruco_msgs::srv::GetMap>(
                    "~/get_map", rclcpp::ServicesQoS(), cb_group);
        } else {
            m_map_client =
                node->template create_client<clover2_aruco_msgs::srv::GetMap>(
                    "~/get_map");
        }

        trigger_map_update();
    }

    bool valid() const { return m_map_valid; }
    const char* get_name() const { return m_name.c_str(); }
    int get_marker_size(int id) const { return m_sizes.at(id); }
    int get_count() const { return m_idx.size(); }
    rclcpp::Time get_last_loaded() const { return m_map_load_time; }

private:
    void map_update_callback(const std_msgs::msg::Empty::SharedPtr /* msg */) {
        trigger_map_update();
    }

    void update_map(const clover2_aruco_msgs::msg::MarkerMap& msg) {
        m_name = msg.name;
        m_map_load_time = rclcpp::Time(msg.map_load_time);

        m_idx.clear();
        m_sizes.clear();

        m_idx.reserve(msg.map.markers.size());
        m_sizes.reserve(msg.map.markers.size());

        for (size_t i = 0; i < msg.map.markers.size(); i++) {
            m_idx[i] = msg.map.markers[i].id;
            m_sizes[i] = msg.map.markers[i].length;
        }

        m_map_valid = true;
    }

    void trigger_map_update() {
        auto map_request = std::make_shared<clover2_aruco_msgs::srv::GetMap::Request>();
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
                            resp->map.name.c_str(), get_name(),
                            resp->map.map.markers.size());

                update_map(resp->map);
            });
    }

    rclcpp::Logger m_logger;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr m_map_update_sub;
    rclcpp::Client<clover2_aruco_msgs::srv::GetMap>::SharedPtr m_map_client;

    bool m_map_valid;
    std::string m_name;
    std::vector<int> m_idx;
    rclcpp::Time m_map_load_time;
    std::unordered_map<int, int> m_sizes;
};

}  // namespace clover2_aruco
