#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    const auto result = RUN_ALL_TESTS();

    rclcpp::shutdown();
    return result;
}
