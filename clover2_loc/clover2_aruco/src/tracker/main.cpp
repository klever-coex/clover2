#include <clover2/aruco/tracker.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor executor;

        auto options = rclcpp::NodeOptions();
        clover2::aruco::tracker::SharedPtr tracker =
            std::make_shared<clover2::aruco::tracker>(options);

        executor.add_node(tracker->get_node_base_interface());
        executor.spin();

    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
