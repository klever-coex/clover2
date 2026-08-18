#pragma once

#include <clover2/map/diagnostics/map_client_task.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_common/rclcpp_trails.hpp>
#include <clover2_pose_msgs/marker.hpp>

// Eigen
#include <Eigen/Geometry>

// ROS2
#include <rclcpp/rclcpp.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

// ROS2 msgs
#include <clover2_pose_msgs/msg/marker.hpp>
#include <clover2_pose_msgs/msg/marker_map.hpp>
#include <clover2_pose_msgs/srv/get_map.hpp>
#include <std_msgs/msg/empty.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

// STL
#include <mutex>
#include <unordered_map>
#include <vector>

namespace clover2::map {

class client {
public:
    explicit client(
        const std::shared_ptr<clover2_common::node_context>& node_context,
        rclcpp::CallbackGroup::SharedPtr cb_group = nullptr)
        : m_logger(node_context->get_logger().get_child("map_client"))
        , m_diagnostics(node_context->get_node_diagnostics_interface())
        , m_map_valid(false)
        , m_name("") {
        rclcpp::SubscriptionOptions options;
        options.callback_group = cb_group;

        m_diagnostics->add<clover2::map::diagnostics::map_client_task>();

        m_diagnostics->get<clover2::map::diagnostics::map_client_task>() //
            .set_name_getter([this]() { return get_name(); });
        m_diagnostics->get<clover2::map::diagnostics::map_client_task>() //
            .set_frame_id_getter([this]() { return get_map_id(); });
        m_diagnostics->get<clover2::map::diagnostics::map_client_task>() //
            .set_marker_count_getter([this]() { return get_count(); });
        m_diagnostics->get<clover2::map::diagnostics::map_client_task>() //
            .set_map_valid_getter([this]() { return valid(); });

        m_map_update_sub = rclcpp::create_subscription<std_msgs::msg::Empty>(
            node_context, "~/map_update",
            rclcpp::QoS(1).transient_local().reliable(),
            std::bind(&client::map_update_callback, this,
                      std::placeholders::_1),
            options);

        m_get_map_client =
            rclcpp::create_client<clover2_pose_msgs::srv::GetMap>(
                node_context, "~/get_map", rclcpp::ServicesQoS());

        update_map();
    }

    ~client() {
        m_map_update_sub.reset();
        m_get_map_client.reset();

        m_diagnostics->remove<clover2::map::diagnostics::map_client_task>();
    }

    bool valid() const { return m_map_valid; }

    std::string get_name() const { return m_name; }

    std::string get_map_id() const { return m_map_id; }

    double get_marker_size(int id) const { return m_markers.at(id).size; }

    int get_count() const { return static_cast<int>(m_markers.size()); }

    const Eigen::Isometry3d& get_transform(int id) const {
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

    void update_cached_map(const clover2_pose_msgs::msg::MarkerMap& msg) {
        std::lock_guard<std::recursive_mutex> guard(m_map_mtx);

        m_name = msg.name;
        m_map_id = msg.header.frame_id;
        m_markers.clear();

        for (const auto& it : msg.markers) {
            clover2_pose_msgs::marker m(it);
            m_markers[it.id] = m;
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
    clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
        m_diagnostics;

    std::recursive_mutex m_map_mtx;

    bool m_map_valid;
    std::string m_name;
    std::string m_map_id;
    std::unordered_map<int, clover2_pose_msgs::marker> m_markers;
};

}  // namespace clover2::map
