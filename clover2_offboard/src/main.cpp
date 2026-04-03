#include <clover2_offboard/server.hpp>

#include <rclcpp/rclcpp.hpp>

#include <memory>

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor executor;

        auto options = rclcpp::NodeOptions();
        clover2_offboard::server::SharedPtr node = std::make_shared<clover2_offboard::server>(options);

        executor.add_node(node->get_node_base_interface());
        executor.spin();
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
