#pragma once

// clover2
#include <clover2_offboard/backend/fabric.hpp>

// ROS2
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

// STL
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace clover2_offboard {

class helper {
public:
    enum state {
        none,
        position,
    };

    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(helper)

    static std::vector<std::string> list_backends();

    template <typename NodeT>
    explicit helper(const NodeT& node, const std::string& backend,
                    const rclcpp::NodeOptions& options = rclcpp::NodeOptions(),
                    rclcpp::CallbackGroup::SharedPtr group = nullptr)
        : m_node(std::make_shared<rclcpp::Node>(
              "offboard_helper", node.get_fully_qualified_name(), options))
        , m_group(group) {
        init(backend);
    }

    ~helper() = default;

    void set_position(const std::string& frame_id, std::optional<double> x,
                      std::optional<double> y, std::optional<double> z,
                      std::optional<double> yaw);

private:
    void init(const std::string& backend);

    void publish_offboard();
    void publish_position();

    rclcpp::Logger get_logger() const { return m_node->get_logger(); }
    rclcpp::Clock::SharedPtr get_clock() const { return m_node->get_clock(); }

    clover2_offboard::backend::base_backend::SharedPtr m_backend;

    state m_state{state::position};
    std::string m_local_frame{"map"};
    std::string m_setpoint_frame{"setpoint"};
    geometry_msgs::msg::PoseStamped m_pose_setpoint;

    rclcpp::Node::SharedPtr m_node;
    std::shared_ptr<tf2_ros::Buffer> m_tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> m_tf_listener;
    rclcpp::CallbackGroup::SharedPtr m_group;
    rclcpp::TimerBase::SharedPtr m_update_timer;
};

}  // namespace clover2_offboard
