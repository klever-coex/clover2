// clover2
#include <clover2/offboard/assistant.hpp>
#include <clover2/offboard/bridge/creation_context.hpp>
#include <clover2/offboard/bridge/fabric.hpp>
#include <clover2/offboard/client.hpp>

// ROS2
#include <clover2_offboard_msgs/srv/move_to.hpp>
#include <clover2_offboard_msgs/srv/set_speed.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/set_bool.hpp>

// STL
#include <memory>

namespace {

class offboard_node : public rclcpp::Node {
public:
    offboard_node() : rclcpp::Node("offboard_node") {
        std::string bridge_type =
            declare_parameter<std::string>("bridge_type", "mavros");

        bridge::creation_context ctx{shared_from_this()};
        m_bridge = bridge::fabric::instance().create(bridge_type, ctx);
        if (!m_bridge) {
            throw std::runtime_error("Failed to create bridge: " + bridge_type);
        }

        m_client = std::make_unique<clover2::offboard::client>(
            shared_from_this(), m_bridge);
        m_assistant = std::make_unique<clover2::offboard::assistant>(
            shared_from_this(), *m_client);

        m_move_to_srv = create_service<clover2_offboard_msgs::srv::MoveTo>(
            "~/move_to",
            std::bind(&offboard_node::move_to_callback, this,
                      std::placeholders::_1, std::placeholders::_2));

        m_set_speed_srv =
            create_service<clover2_offboard_msgs::srv::SetSpeed>(
                "~/set_speed",
                std::bind(&offboard_node::set_speed_callback, this,
                          std::placeholders::_1, std::placeholders::_2));

        m_arm_srv = create_service<std_srvs::srv::SetBool>(
            "~/arm",
            std::bind(&offboard_node::arm_callback, this,
                      std::placeholders::_1, std::placeholders::_2));

        m_disarm_srv = create_service<std_srvs::srv::SetBool>(
            "~/disarm",
            std::bind(&offboard_node::disarm_callback, this,
                      std::placeholders::_1, std::placeholders::_2));

        RCLCPP_INFO(get_logger(), "offboard_node started");
    }

private:
    void move_to_callback(
        const clover2_offboard_msgs::srv::MoveTo::Request::SharedPtr request,
        clover2_offboard_msgs::srv::MoveTo::Response::SharedPtr response) {
        std::string frame_id = request->frame_id.empty() ? "map" : request->frame_id;
        m_assistant->set_position_setpoint(
            request->position.x, request->position.y, request->position.z,
            request->yaw, frame_id);
        response->success = true;
    }

    void set_speed_callback(
        const clover2_offboard_msgs::srv::SetSpeed::Request::SharedPtr request,
        clover2_offboard_msgs::srv::SetSpeed::Response::SharedPtr response) {
        m_assistant->set_speed(request->speed);
        response->success = true;
    }

    void arm_callback(
        const std_srvs::srv::SetBool::Request::SharedPtr,
        std_srvs::srv::SetBool::Response::SharedPtr response) {
        m_client->arm();
        response->success = true;
    }

    void disarm_callback(
        const std_srvs::srv::SetBool::Request::SharedPtr,
        std_srvs::srv::SetBool::Response::SharedPtr response) {
        m_client->disarm();
        response->success = true;
    }

    std::shared_ptr<bridge::base_bridge> m_bridge;
    std::unique_ptr<clover2::offboard::client> m_client;
    std::unique_ptr<clover2::offboard::assistant> m_assistant;

    rclcpp::Service<clover2_offboard_msgs::srv::MoveTo>::SharedPtr m_move_to_srv;
    rclcpp::Service<clover2_offboard_msgs::srv::SetSpeed>::SharedPtr
        m_set_speed_srv;
    rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr m_arm_srv;
    rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr m_disarm_srv;
};

}  // namespace

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    try {
        auto node = std::make_shared<offboard_node>();
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(rclcpp::get_logger("offboard_node"), "Error: %s",
                     e.what());
        return 1;
    }
    rclcpp::shutdown();
    return 0;
}
