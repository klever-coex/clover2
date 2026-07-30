#include <clover2_display/driver.hpp>

#include <rclcpp/rclcpp.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    try {
        rclcpp::executors::SingleThreadedExecutor executor;

        auto options = rclcpp::NodeOptions();
        clover2_display::driver::SharedPtr driver =
            std::make_shared<clover2_display::driver>(options);

        executor.add_node(driver->get_node_base_interface());
        executor.spin();
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught an exception: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}
