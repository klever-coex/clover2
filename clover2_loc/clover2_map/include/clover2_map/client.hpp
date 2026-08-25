#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_map/data/map.hpp>
#include <clover2_map/diagnostics/map_client_task.hpp>

// ROS2
#include <rclcpp/create_client.hpp>
#include <rclcpp/create_subscription.hpp>
#include <rclcpp/rclcpp.hpp>

// ROS2 msgs
#include <clover2_pose_msgs/msg/marker_map.hpp>
#include <clover2_pose_msgs/srv/get_map.hpp>
#include <clover2_pose_msgs/srv/modify_map.hpp>

#include <std_msgs/msg/empty.hpp>

// STL
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

namespace clover2_map {

class client {
public:
    using modify_callback =
        std::function<void(bool success, std::string error_message)>;

    client(
        std::shared_ptr<rclcpp::node_interfaces::NodeBaseInterface> node_base,
        std::shared_ptr<rclcpp::node_interfaces::NodeGraphInterface> node_graph,
        std::shared_ptr<rclcpp::node_interfaces::NodeParametersInterface>
            node_parameters,
        std::shared_ptr<rclcpp::node_interfaces::NodeTopicsInterface>
            node_topics,
        std::shared_ptr<rclcpp::node_interfaces::NodeServicesInterface>
            node_services,
        std::shared_ptr<rclcpp::node_interfaces::NodeLoggingInterface>
            node_logging,
        std::shared_ptr<
            clover2_common::node_interfaces::NodeDiagnosticsInterface>
            node_diagnostics,
        rclcpp::CallbackGroup::SharedPtr cb_group = nullptr,
        std::string map_node_name = "")
        : m_logger(node_logging->get_logger().get_child("map_client"))
        , m_diagnostics(std::move(node_diagnostics))
        , m_map_valid(false) {
        const std::string prefix =
            map_node_name.empty() ? "~/" : (map_node_name + "/");

        rclcpp::SubscriptionOptions options;
        options.callback_group = cb_group;

        m_diagnostics->add<diagnostics::map_client_task>();

        m_diagnostics
            ->get<diagnostics::map_client_task>()  //
            .set_name_getter([this]() { return get_name(); });
        m_diagnostics
            ->get<diagnostics::map_client_task>()  //
            .set_frame_id_getter([this]() { return get_map_id(); });
        m_diagnostics
            ->get<diagnostics::map_client_task>()  //
            .set_marker_count_getter([this]() { return get_count(); });
        m_diagnostics
            ->get<diagnostics::map_client_task>()  //
            .set_map_valid_getter([this]() { return valid(); });

        m_map_update_sub = rclcpp::create_subscription<std_msgs::msg::Empty>(
            node_parameters, node_topics, prefix + "map_update",
            rclcpp::QoS(1).transient_local().reliable(),
            std::bind(&client::map_update_callback, this,
                      std::placeholders::_1),
            options);

        m_get_map_client =
            rclcpp::create_client<clover2_pose_msgs::srv::GetMap>(
                node_base, node_graph, node_services, prefix + "get_map",
                rclcpp::ServicesQoS(), cb_group);

        m_modify_map_client =
            rclcpp::create_client<clover2_pose_msgs::srv::ModifyMap>(
                node_base, node_graph, node_services, prefix + "modify_map",
                rclcpp::ServicesQoS(), cb_group);

        update_map();
    }

    template <typename NodeT>
    explicit client(const NodeT& node,
                    rclcpp::CallbackGroup::SharedPtr cb_group = nullptr,
                    std::string map_node_name = "")
        : client(node->get_node_base_interface(),
                 node->get_node_graph_interface(),
                 node->get_node_parameters_interface(),
                 node->get_node_topics_interface(),
                 node->get_node_services_interface(),
                 node->get_node_logging_interface(),
                 node->get_node_diagnostics_interface(), cb_group,
                 std::move(map_node_name)) {}

    ~client() {
        m_map_update_sub.reset();
        m_get_map_client.reset();

        m_diagnostics->remove<diagnostics::map_client_task>();
    }

    bool valid() const { return m_map_valid; }

    map snapshot() const {
        std::lock_guard<std::recursive_mutex> guard(m_map_mtx);
        return m_map;
    }

    const map& get_map() const { return m_map; }

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

    void refresh() { update_map(); }

    void modify_marker(uint8_t operation, const clover2_map::marker& mk,
                       modify_callback callback = {}) {
        auto fail = [&](const std::string& message) {
            if (callback) {
                callback(false, message);
            } else {
                RCLCPP_ERROR(m_logger, "Fail to modify map: %s",
                             message.c_str());
            }
        };

        if (!m_modify_map_client->service_is_ready()) {
            fail("modify_map service is not available");
            return;
        }

        auto request =
            std::make_shared<clover2_pose_msgs::srv::ModifyMap::Request>();
        request->operation = operation;
        mk.to_msg(request->marker);

        m_modify_map_client->async_send_request(
            request,
            [this, callback](
                rclcpp::Client<clover2_pose_msgs::srv::ModifyMap>::SharedFuture
                    future) {
                try {
                    if (!future.valid()) {
                        if (callback) {
                            callback(false, "Invalid future");
                        } else {
                            RCLCPP_ERROR(m_logger, "Fail to modify map");
                        }
                        return;
                    }

                    auto resp = future.get();
                    if (callback) {
                        callback(resp->success, resp->error_message);
                    } else if (!resp->success) {
                        RCLCPP_ERROR(m_logger, "Fail to modify map: %s",
                                     resp->error_message.c_str());
                    }
                } catch (const std::exception& e) {
                    if (callback) {
                        callback(false, e.what());
                    } else {
                        RCLCPP_ERROR(m_logger, "Fail to modify map: %s",
                                     e.what());
                    }
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
        if (!m_get_map_client->service_is_ready()) {
            RCLCPP_WARN(m_logger, "%s service is not available",
                        m_get_map_client->get_service_name());
            return;
        }

        auto map_request =
            std::make_shared<clover2_pose_msgs::srv::GetMap::Request>();
        m_get_map_client->async_send_request(
            map_request,
            [this](rclcpp::Client<clover2_pose_msgs::srv::GetMap>::SharedFuture
                       future) {
                try {
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
                } catch (const std::exception& e) {
                    RCLCPP_ERROR(m_logger, "Fail to get map: %s", e.what());
                }
            });
    }

    rclcpp::Logger m_logger;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr m_map_update_sub;
    rclcpp::Client<clover2_pose_msgs::srv::GetMap>::SharedPtr m_get_map_client;
    rclcpp::Client<clover2_pose_msgs::srv::ModifyMap>::SharedPtr
        m_modify_map_client;
    clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
        m_diagnostics;

    mutable std::recursive_mutex m_map_mtx;

    bool m_map_valid;
    clover2_map::map m_map;
    std::unordered_map<int, clover2_map::marker> m_markers;
};

}  // namespace clover2_map
