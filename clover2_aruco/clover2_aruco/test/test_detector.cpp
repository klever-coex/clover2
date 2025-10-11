// GTest
#include <gtest/gtest.h>

// OpenCV
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>
#include <clover2_aruco/detector.hpp>

using namespace clover2_aruco;

class DetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        rclcpp::NodeOptions options;
        detector_node = std::make_shared<detector>(options);
    }

    std::shared_ptr<detector> detector_node;
};

// Test parameter setting
TEST_F(DetectorTest, SetParametersValidDict) {
    rclcpp::Parameter param("marker_dict", "5X5_100");
    std::vector<rclcpp::Parameter> params = {param};
    auto res = detector_node->on_set_parameters_cb(params);
    EXPECT_TRUE(res.successful);
}

// Test invalid dictionary parameter
TEST_F(DetectorTest, SetParametersInvalidDict) {
    rclcpp::Parameter param("marker_dict", "INVALID_DICT");
    std::vector<rclcpp::Parameter> params = {param};
    auto res = detector_node->on_set_parameters_cb(params);
    EXPECT_FALSE(res.successful);
}

// Test marker object points generation for CW pattern
TEST_F(DetectorTest, MarkerObjectPointsCW) {
    auto est_params = cv::makePtr<cv::aruco::EstimateParameters>();
    est_params->pattern = cv::aruco::CW_top_left_corner;
    double size = 0.1;
    cv::Mat objPoints = detector_node->marker_object_points(size, est_params);

    EXPECT_EQ(objPoints.rows, 4);
    EXPECT_EQ(objPoints.cols, 1);

    cv::Vec3f first = objPoints.ptr<cv::Vec3f>(0)[0];
    EXPECT_FLOAT_EQ(first[0], 0.0);
    EXPECT_FLOAT_EQ(first[1], 0.0);
}

// Test marker object points generation for CCW pattern
TEST_F(DetectorTest, MarkerObjectPointsCCW) {
    auto est_params = cv::makePtr<cv::aruco::EstimateParameters>();
    est_params->pattern = cv::aruco::CCW_center;
    double size = 0.2;
    cv::Mat objPoints = detector_node->marker_object_points(size, est_params);

    cv::Vec3f first = objPoints.ptr<cv::Vec3f>(0)[0];
    EXPECT_FLOAT_EQ(first[0], -0.1);
    EXPECT_FLOAT_EQ(first[1], 0.1);
}

// Test pose filling
TEST_F(DetectorTest, FillPose) {
    clover2_aruco_msgs::msg::Marker marker;
    cv::Vec3d rvec(0.0, 0.0, M_PI/2);  // 90 deg rotation around Z
    cv::Vec3d tvec(1.0, 2.0, 3.0);

    detector_node->fill_pose(marker, rvec, tvec);

    EXPECT_DOUBLE_EQ(marker.pose.position.x, 1.0);
    EXPECT_DOUBLE_EQ(marker.pose.position.y, 2.0);
    EXPECT_DOUBLE_EQ(marker.pose.position.z, 3.0);

    double norm = std::sqrt(marker.pose.orientation.x * marker.pose.orientation.x +
                            marker.pose.orientation.y * marker.pose.orientation.y +
                            marker.pose.orientation.z * marker.pose.orientation.z +
                            marker.pose.orientation.w * marker.pose.orientation.w);
    EXPECT_NEAR(norm, 1.0, 1e-6);  // Quaternion should be normalized
}

// Test frame ID generation
TEST_F(DetectorTest, MarkerFrameID) {
    rclcpp::Parameter frame_param("marker_frame_id", "aruco_test_");
    detector_node->on_set_parameters_cb({frame_param});
    std::string frame_id = detector_node->get_marker_frame_id(42);
    EXPECT_EQ(frame_id, "aruco_test_42");
}

