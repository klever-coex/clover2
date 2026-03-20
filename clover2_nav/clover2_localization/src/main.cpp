// clover2
#include <clover2/localization/localization.hpp>

// ROS2
#include <lifecycle_msgs/msg/state.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor executor;

        auto options = rclcpp::NodeOptions();
        options.append_parameter_override("autostart", false);
        auto node =
            std::make_shared<clover2::localization::localization>(options);

        executor.add_node(node->get_node_base_interface());
        node->configure();
        if (node->get_current_state().id() ==
            lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE) {
            node->activate();
        }
        auto sensor_if = node->get_sensor_node_base_interface();
        if (sensor_if) {
            executor.add_node(sensor_if);
        }
        executor.spin();

    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
