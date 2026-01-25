#include <clover2/optical_flow/optical_flow.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor executor;

        auto options = rclcpp::NodeOptions();
        clover2::optical_flow::optical_flow::SharedPtr optical_flow_node =
            std::make_shared<clover2::optical_flow::optical_flow>(options);

        executor.add_node(optical_flow_node->get_node_base_interface());
        executor.spin();

    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}

