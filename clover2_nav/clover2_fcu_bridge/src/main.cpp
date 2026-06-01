#include <clover2_fcu_bridge/server.hpp>
#include <rclcpp/executors/multi_threaded_executor.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor::SharedPtr executor =
            std::make_shared<rclcpp::executors::SingleThreadedExecutor>();

        auto options = rclcpp::NodeOptions();
        clover2_fcu_bridge::server::SharedPtr node =
            std::make_shared<clover2_fcu_bridge::server>(options);

        executor->add_node(node->get_node_base_interface());
        executor->spin();
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
