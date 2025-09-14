#include <rclcpp/rclcpp.hpp>

#include <clover2_aruco/map_server.hpp>

#include <memory>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor executor;

        auto options = rclcpp::NodeOptions();
        clover2_aruco::detector::SharedPtr detector =
            std::make_shared<clover2_aruco::detector>(options);

        executor.add_node(detector->get_node_base_interface());
        executor.spin();

    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
