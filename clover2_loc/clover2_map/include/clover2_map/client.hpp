#pragma once

// clover2
#include <clover2_map/data/map.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// ROS2 msgs
#include <clover2_pose_msgs/msg/marker_map.hpp>
#include <clover2_pose_msgs/srv/get_map.hpp>
#include <clover2_pose_msgs/srv/modify_map.hpp>

#include <std_msgs/msg/empty.hpp>

// STL
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace clover2_map {

class client {
public:
    template <typename NodeT>
    explicit client(const NodeT& node,
                    rclcpp::CallbackGroup::SharedPtr cb_group = nullptr)
        : m_logger(node->get_logger().get_child("map_client"))
        , m_map_valid(false) {
        rclcpp::SubscriptionOptions options;
        options.callback_group = cb_group;

        m_map_update_sub =
            node->template create_subscription<std_msgs::msg::Empty>(
                "~/map_update", rclcpp::QoS(1).transient_local().reliable(),
                std::bind(&client::map_update_callback, this,
                          std::placeholders::_1),
                options);

        m_get_map_client =
            node->template create_client<clover2_pose_msgs::srv::GetMap>(
                "~/get_map", rclcpp::ServicesQoS());

        m_modify_map_client =
            node->template create_client<clover2_pose_msgs::srv::ModifyMap>(
                "~/modify_map", rclcpp::ServicesQoS());

        update_map();
    }

    bool valid() const { return m_map_valid; }

    const clover2_map::map& get_map() const { return m_map; }

    std::string get_name() const { return m_map.name; }

    std::string get_map_id() const { return m_map.frame_id; }

    std::string get_dictionary() const { return m_map.dictionary; }

    int get_count() const { return static_cast<int>(m_markers.size()); }

    bool has_marker(int id) const {
        return m_markers.find(id) != m_markers.end();
    }

    const clover2_map::marker& get_marker(int id) const {
        return m_markers.at(id);
    }

    void modify_marker(
        uint8_t operation, const clover2_map::marker& mk) {
        if (!m_modify_map_client->wait_for_service(
                std::chrono::milliseconds(1000))) {
            RCLCPP_ERROR(m_logger, "%s service is not available!",
                         m_modify_map_client->get_service_name());
            return;
        }

        auto request =
            std::make_shared<clover2_pose_msgs::srv::ModifyMap::Request>();
        request->operation = operation;
        mk.to_msg(request->marker);

        m_modify_map_client->async_send_request(
            request,
            [this](rclcpp::Client<
                   clover2_pose_msgs::srv::ModifyMap>::SharedFuture future) {
                if (!future.valid()) {
                    RCLCPP_ERROR(m_logger, "Fail to modify map");
                    return;
                }

                auto resp = future.get();
                if (!resp->success) {
                    RCLCPP_ERROR(m_logger, "Fail to modify map: %s",
                                 resp->error_message.c_str());
                }
            });
    }

private:
    void map_update_callback(const std_msgs::msg::Empty::SharedPtr /* msg */) {
        update_map();
    }

    void update_cached_map(const clover2_pose_msgs::msg::MarkerMap& msg) {
        std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

        m_map = clover2_map::map::from_msg(msg);

        m_markers.clear();
        m_markers.reserve(m_map.markers.size());
        for (const auto& mk : m_map.markers) {
            m_markers.emplace(mk.id, mk);
        }

        m_map_valid = true;
    }

    void update_map() {
        if (!m_get_map_client->wait_for_service(
                std::chrono::milliseconds(1000))) {
            throw std::runtime_error(
                std::string(m_get_map_client->get_service_name()) +
                " service is not available!");
        }

        auto map_request =
            std::make_shared<clover2_pose_msgs::srv::GetMap::Request>();
        m_get_map_client->async_send_request(
            map_request,
            [this](rclcpp::Client<clover2_pose_msgs::srv::GetMap>::SharedFuture
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
    rclcpp::Client<clover2_pose_msgs::srv::GetMap>::SharedPtr m_get_map_client;
    rclcpp::Client<clover2_pose_msgs::srv::ModifyMap>::SharedPtr
        m_modify_map_client;

    std::recursive_mutex m_map_mtx;

    bool m_map_valid;
    clover2_map::map m_map;
    std::unordered_map<int, clover2_map::marker> m_markers;
};

}  // namespace clover2_map
