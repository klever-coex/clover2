// testing
#include <gtest/gtest.h>

// OpenCV
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>

// ROS2
#include <clover2_aruco/detector.hpp>
#include <rclcpp/rclcpp.hpp>

class DetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        rclcpp::NodeOptions options;
        options.automatically_declare_parameters_from_overrides(true);
        detector_node = std::make_shared<clover2_aruco::detector>(options);
    }

    void TearDown() override {
        detector_node.reset();
    }

    clover2_aruco::detector::SharedPtr detector_node;
};

// Test parameter setting
TEST_F(DetectorTest, SetParametersValidDict) {
    rclcpp::Parameter param("marker_dict", "5X5_100");
    std::vector<rclcpp::Parameter> params = {param};
    auto res = detector_node->on_set_parameters_cb(params);
    EXPECT_TRUE(res.successful);
}

// Test pose filling
TEST_F(DetectorTest, FillPose) {
    clover2_aruco_msgs::msg::Marker marker;
    cv::Vec3d rvec(0.0, 0.0, M_PI / 2);  // 90 deg rotation around Z
    cv::Vec3d tvec(1.0, 2.0, 3.0);

    detector_node->fill_pose(marker, rvec, tvec);

    EXPECT_DOUBLE_EQ(marker.pose.position.x, 1.0);
    EXPECT_DOUBLE_EQ(marker.pose.position.y, 2.0);
    EXPECT_DOUBLE_EQ(marker.pose.position.z, 3.0);

    double norm =
        std::sqrt(marker.pose.orientation.x * marker.pose.orientation.x +
                  marker.pose.orientation.y * marker.pose.orientation.y +
                  marker.pose.orientation.z * marker.pose.orientation.z +
                  marker.pose.orientation.w * marker.pose.orientation.w);
    EXPECT_NEAR(norm, 1.0, 1e-6);
}

// Test frame ID generation
TEST_F(DetectorTest, MarkerFrameID) {
    rclcpp::Parameter frame_param("marker_frame_id", "aruco_test_");
    auto result = detector_node->on_set_parameters_cb({frame_param});
    EXPECT_TRUE(result.successful);

    std::string frame_id = detector_node->get_marker_frame_id(42);
    EXPECT_EQ(frame_id, "aruco_test_42");
}

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();
    rclcpp::shutdown();

    return result;
}
