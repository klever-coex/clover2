#include <clover2_notification/controller.hpp>
#include <rclcpp/rclcpp.hpp>

#include <exception>
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::MultiThreadedExecutor executor;
        auto controller = std::make_shared<clover2_notification::controller>();
        executor.add_node(controller->get_node_base_interface());
        executor.spin();
    } catch (const std::exception& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
