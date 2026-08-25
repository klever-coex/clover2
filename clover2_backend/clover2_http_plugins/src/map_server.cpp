// clover2
#include <clover2_common/util/parameter.hpp>
#include <clover2_http_plugins/map_server.hpp>
#include <clover2_pose_msgs/srv/modify_map.hpp>

// tf2
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <tf2/LinearMath/Quaternion.hpp>

// STL
#include <string>
#include <utility>

namespace clover2_http_plugins {

namespace {

namespace http = clover2_http::http;
using clover2_http_plugins::data::marker_info;
using clover2_http_plugins::data::marker_pose;
using clover2_http_plugins::data::modify_result;

data::marker_pose to_pose(const Eigen::Isometry3d& pose) {
    Eigen::Quaterniond eq(pose.linear());
    tf2::Quaternion q(eq.x(), eq.y(), eq.z(), eq.w());
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 0.0;
    tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);

    const auto& t = pose.translation();
    return {t.x(), t.y(), t.z(), roll, pitch, yaw};
}

Eigen::Isometry3d from_pose(const marker_pose& p) {
    Eigen::Isometry3d pose = Eigen::Isometry3d::Identity();
    pose.translation() = Eigen::Vector3d(p.x, p.y, p.z);

    tf2::Quaternion q;
    q.setRPY(p.roll, p.pitch, p.yaw);
    pose.linear() =
        Eigen::Quaterniond(q.w(), q.x(), q.y(), q.z()).toRotationMatrix();

    return pose;
}

marker_info to_info(const clover2_map::marker& mk) {
    marker_info info;
    info.id = mk.id;
    info.type = mk.type.to_string();
    info.size = mk.size;
    info.marker_frame_id = mk.marker_frame_id;
    if (mk.pose) {
        info.pose = to_pose(*mk.pose);
    }

    return info;
}

clover2_map::marker from_info(const marker_info& info) {
    if (info.id < 0) {
        throw http::core::http_error(400, "Marker id must be non-negative");
    }

    if (info.size <= 0.0) {
        throw http::core::http_error(400, "Marker size must be positive");
    }

    clover2_map::marker mk;
    mk.id = info.id;
    mk.size = info.size;
    mk.marker_frame_id = info.marker_frame_id;

    try {
        mk.type = clover2_map::marker_type::from_string(info.type);
    } catch (const std::exception& e) {
        throw http::core::http_error(400, e.what());
    }

    if (mk.type == clover2_map::marker_type::fixed) {
        if (!info.pose) {
            throw http::core::http_error(400, "Fixed markers require a pose");
        }
        mk.pose = from_pose(*info.pose);
    } else if (info.pose) {
        throw http::core::http_error(400, "Only fixed markers may have a pose");
    }

    return mk;
}

}  // namespace

void map_server::on_initialize() {
    using clover2_http::http::core::request_context;

    m_map_group =
        m_node_context->get_node_base_interface()->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive);

    auto parameters = m_node_context->get_node_parameters_interface();
    clover2_common::util::declare_parameter_if_not_declared(
        parameters, "map_node", "map_server");

    rclcpp::Parameter map_node_param;
    parameters->get_parameter("map_node", map_node_param);
    std::string map_node = map_node_param.as_string();
    if (!map_node.empty() && map_node.front() != '/') {
        map_node.insert(map_node.begin(), '/');
    }

    m_map_client = std::make_shared<clover2_map::client>(m_node_context,
                                                         m_map_group, map_node);

    m_refresh_timer = rclcpp::create_timer(
        m_node_context->get_node_base_interface(),
        m_node_context->get_node_timers_interface(),
        m_node_context->get_node_clock_interface()->get_clock(),
        rclcpp::Duration::from_seconds(2.0),
        [this]() {
            if (m_map_client && !m_map_client->valid()) {
                m_map_client->refresh();
            }
        },
        m_map_group);

    m_server->get<data::map_info>(
        "/api/map", std::bind(&map_server::handle_map, this,
                              std::placeholders::_1, std::placeholders::_2));

    m_server->get<data::marker_info>(
        "/api/map/marker/-/{id}",
        std::bind(&map_server::handle_marker, this, std::placeholders::_1,
                  std::placeholders::_2));

    m_server->post<data::marker_info, data::modify_result>(
        "/api/map/marker",
        std::bind(&map_server::handle_add, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));

    m_server->put<data::marker_info, data::modify_result>(
        "/api/map/marker/-/{id}",
        std::bind(&map_server::handle_edit, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));

    m_server->del<void, data::modify_result>(
        "/api/map/marker/-/{id}",
        std::bind(&map_server::handle_delete, this, std::placeholders::_1,
                  std::placeholders::_2));
}

std::vector<std::string> map_server::capabilities() const { return {"map"}; }

void map_server::handle_map(
    http::core::request_context /*ctx*/,
    http::endpoint::deferred_reply<data::map_info> reply) {
    if (!m_map_client) {
        reply.error_json(503, "Map client is not initialized");
        return;
    }

    const auto map = m_map_client->snapshot();

    data::map_info info;
    info.valid = m_map_client->valid();
    if (info.valid) {
        info.name = map.name;
        info.frame_id = map.frame_id;
        info.dictionary = map.dictionary;
        info.count = static_cast<int>(map.markers.size());
        info.markers.reserve(map.markers.size());

        for (const auto& mk : map.markers) {
            info.markers.push_back(to_info(mk));
        }
    }

    reply(info, 200);
}

void map_server::handle_marker(
    http::core::request_context ctx,
    http::endpoint::deferred_reply<data::marker_info> reply) {
    const int id = ctx.param<int>("id");

    if (!m_map_client || !m_map_client->valid()) {
        reply.error_json(503, "Map is not available");
        return;
    }

    const auto map = m_map_client->snapshot();
    for (const auto& mk : map.markers) {
        if (mk.id == id) {
            reply(to_info(mk), 200);
            return;
        }
    }

    reply.error_json(404, "Marker not found");
}

void map_server::handle_add(
    http::core::request_context /*ctx*/, data::marker_info request,
    http::endpoint::deferred_reply<data::modify_result> reply) {
    clover2_map::marker mk;
    try {
        mk = from_info(request);
    } catch (const http::core::http_error& e) {
        reply.error_json(e.status(), e.message());
        return;
    }

    auto timer = std::make_shared<rclcpp::TimerBase::SharedPtr>();
    *timer = rclcpp::create_timer(
        m_node_context->get_node_base_interface(),
        m_node_context->get_node_timers_interface(),
        m_node_context->get_node_clock_interface()->get_clock(),
        rclcpp::Duration::from_seconds(5.0),
        [reply, timer]() {
            (*timer)->cancel();
            reply(modify_result{false, "timeout"}, 504);
        },
        m_map_group);

    m_map_client->modify_marker(
        clover2_pose_msgs::srv::ModifyMap::Request::OPERATION_ADD, mk,
        [reply, timer](bool success, std::string error_message) {
            (*timer)->cancel();
            reply(modify_result{success, std::move(error_message)});
        });
}

void map_server::handle_edit(
    http::core::request_context ctx, data::marker_info request,
    http::endpoint::deferred_reply<data::modify_result> reply) {
    const int id = ctx.param<int>("id");

    if (request.id != id) {
        RCLCPP_WARN(get_logger(),
                    "PUT marker: body id %d overridden by path id %d",
                    request.id, id);
        request.id = id;
    }

    clover2_map::marker mk;
    try {
        mk = from_info(request);
    } catch (const http::core::http_error& e) {
        reply.error_json(e.status(), e.message());
        return;
    }

    auto timer = std::make_shared<rclcpp::TimerBase::SharedPtr>();
    *timer = rclcpp::create_timer(
        m_node_context->get_node_base_interface(),
        m_node_context->get_node_timers_interface(),
        m_node_context->get_node_clock_interface()->get_clock(),
        rclcpp::Duration::from_seconds(5.0),
        [reply, timer]() {
            (*timer)->cancel();
            reply(modify_result{false, "timeout"}, 504);
        },
        m_map_group);

    m_map_client->modify_marker(
        clover2_pose_msgs::srv::ModifyMap::Request::OPERATION_EDIT, mk,
        [reply, timer](bool success, std::string error_message) {
            (*timer)->cancel();
            reply(modify_result{success, std::move(error_message)});
        });
}

void map_server::handle_delete(
    http::core::request_context ctx,
    http::endpoint::deferred_reply<data::modify_result> reply) {
    const int id = ctx.param<int>("id");

    clover2_map::marker mk;
    mk.id = id;

    auto timer = std::make_shared<rclcpp::TimerBase::SharedPtr>();
    *timer = rclcpp::create_timer(
        m_node_context->get_node_base_interface(),
        m_node_context->get_node_timers_interface(),
        m_node_context->get_node_clock_interface()->get_clock(),
        rclcpp::Duration::from_seconds(5.0),
        [reply, timer]() {
            (*timer)->cancel();
            reply(modify_result{false, "timeout"}, 504);
        },
        m_map_group);

    m_map_client->modify_marker(
        clover2_pose_msgs::srv::ModifyMap::Request::OPERATION_DELETE, mk,
        [reply, timer](bool success, std::string error_message) {
            (*timer)->cancel();
            reply(modify_result{success, std::move(error_message)});
        });
}

}  // namespace clover2_http_plugins

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_http_plugins::map_server,
                       clover2_http::base_plugin)
