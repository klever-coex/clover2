#pragma once

#include <Eigen/Geometry>
#include <rclcpp/rclcpp.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

#include <clover2_aruco_msgs/msg/marker.hpp>
#include <clover2_aruco_msgs/msg/marker_map.hpp>
#include <std_msgs/msg/empty.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <clover2_aruco_msgs/srv/get_map.hpp>

#include <mutex>
#include <unordered_map>
#include <vector>

namespace clover2::map_server {

class map_client {
public:
    struct marker {
        marker()
            : id(0)
            , size(0.0)
            , transform()
            , marker_frame_id() {}

        marker(const clover2_aruco_msgs::msg::Marker& msg)
            : id(msg.id)
            , size(msg.size)
            , marker_frame_id(msg.marker_frame_id) {
            tf2::fromMsg(msg.pose.pose, transform);
        }

        marker(const marker& other)
            : id(other.id)
            , size(other.size)
            , transform(other.transform)
            , marker_frame_id(other.marker_frame_id) {}

        marker& operator=(const marker& other) {
            if (this != &other) {
                id = other.id;
                size = other.size;
                transform = other.transform;
                marker_frame_id = other.marker_frame_id;
            }

            return *this;
        }

        int id;
        double size;
        Eigen::Affine3d transform;
        std::string marker_frame_id;
    };

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
            m_get_map_client =
                node->template create_client<clover2_aruco_msgs::srv::GetMap>(
                    "~/get_map", rclcpp::ServicesQoS(), cb_group);
        } else {
            m_get_map_client =
                node->template create_client<clover2_aruco_msgs::srv::GetMap>(
                    "~/get_map", rclcpp::ServicesQoS());
        }

        update_map();
    }

    bool valid() const { return m_map_valid; }

    std::string get_name() const { return m_name; }

    std::string get_map_id() const { return m_map_id; }

    double get_marker_size(int id) const { return m_markers.at(id).size; }

    int get_count() const { return static_cast<int>(m_markers.size()); }

    const Eigen::Affine3d& get_transform(int id) const {
        return m_markers.at(id).transform;
    }

    const std::string& get_marker_frame_id(int id) const {
        return m_markers.at(id).marker_frame_id;
    }

    bool has_marker(int id) const {
        return m_markers.find(id) != m_markers.end();
    }

private:
    void map_update_callback(const std_msgs::msg::Empty::SharedPtr /* msg */) {
        update_map();
    }

    void update_cached_map(const clover2_aruco_msgs::msg::MarkerMap& msg) {
        std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

        m_name = msg.name;
        m_map_id = msg.header.frame_id;
        m_markers.clear();

        for (const auto& it : msg.markers) {
            marker m(it);
            m_markers[it.id] = m;
        }

        m_map_valid = true;
    }

    void update_map() {
        if (!m_get_map_client->wait_for_service(std::chrono::milliseconds(1000))) {
            throw std::runtime_error(
                std::string(m_get_map_client->get_service_name()) +
                " service is not available!");
        }

        auto map_request =
            std::make_shared<clover2_aruco_msgs::srv::GetMap::Request>();
        m_get_map_client->async_send_request(
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
                            get_name().c_str(), resp->map.name.c_str(),
                            resp->map.markers.size());

                update_cached_map(resp->map);
            });
    }

    rclcpp::Logger m_logger;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr m_map_update_sub;
    rclcpp::Client<clover2_aruco_msgs::srv::GetMap>::SharedPtr m_get_map_client;

    std::recursive_mutex m_map_mtx;

    bool m_map_valid;
    std::string m_name;
    std::string m_map_id;
    std::unordered_map<int, marker> m_markers;
};

}  // namespace clover2::map_server
