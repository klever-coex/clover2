// clover2
#include <clover2/aruco/map_server.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor executor;

        auto options = rclcpp::NodeOptions();
        clover2::aruco::map_server::SharedPtr map_server =
            std::make_shared<clover2::aruco::map_server>(options);

        executor.add_node(map_server->get_node_base_interface());
        executor.spin();

    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
