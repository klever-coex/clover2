#include <clover2/cam_feature/cam_feature.hpp>
#include <clover2_pose_msgs/srv/get_map.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>

namespace {

using namespace std::chrono_literals;

constexpr const char* kCamFeatureStatusName = "Cam feature status";
constexpr int kImageWidth = 64;
constexpr int kImageHeight = 48;

class CamFeatureDiagnosticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_node_ = std::make_shared<rclcpp::Node>("cam_feature_diag_test");

        // cam_feature creates clover2::map::client during configure(), and the
        // client synchronously waits for this private service to exist.
        map_service_ =
            test_node_->create_service<clover2_pose_msgs::srv::GetMap>(
                "/cam_feature/get_map",
                [](const clover2_pose_msgs::srv::GetMap::Request::SharedPtr,
                   clover2_pose_msgs::srv::GetMap::Response::SharedPtr
                       response) {
                    // The diagnostics only need the map to become valid; marker
                    // contents are irrelevant for these tests.
                    response->map.header.frame_id = "map";
                    response->map.name = "test_map";
                });

        // Test the public ROS diagnostics stream instead of calling the private
        // produce_diagnostics() method directly.
        diagnostics_sub_ = test_node_->create_subscription<
            diagnostic_msgs::msg::DiagnosticArray>(
            "/diagnostics", rclcpp::QoS(10),
            [this](diagnostic_msgs::msg::DiagnosticArray::ConstSharedPtr msg) {
                last_diagnostics_ = *msg;
            });

        image_pub_ = test_node_->create_publisher<sensor_msgs::msg::Image>(
            "/cam_feature/input/image_raw", rclcpp::SensorDataQoS());
        camera_info_pub_ =
            test_node_->create_publisher<sensor_msgs::msg::CameraInfo>(
                "/cam_feature/input/camera_info", rclcpp::SensorDataQoS());

        cam_feature_ = std::make_shared<clover2::cam_feature::cam_feature>(
            rclcpp::NodeOptions());

        // cam_feature autostarts from a zero-delay timer, so spinning this
        // executor drives configure(), activate(), service replies, topic
        // callbacks, and diagnostics delivery.
        executor_.add_node(test_node_);
        executor_.add_node(cam_feature_->get_node_base_interface());
    }

    void TearDown() override {
        executor_.remove_node(cam_feature_->get_node_base_interface());
        executor_.remove_node(test_node_);

        cam_feature_.reset();
        test_node_.reset();
        last_diagnostics_.reset();
    }

    bool waitUntil(const std::function<bool()>& predicate,
                   const std::chrono::milliseconds timeout) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            executor_.spin_some();
            if (predicate()) {
                return true;
            }
            std::this_thread::sleep_for(10ms);
        }
        return false;
    }

    sensor_msgs::msg::Image makeImage() {
        // A black mono image is enough: these tests verify diagnostics plumbing,
        // not marker detection quality.
        sensor_msgs::msg::Image msg;
        msg.header.stamp = test_node_->get_clock()->now();
        msg.header.frame_id = "camera";
        msg.height = kImageHeight;
        msg.width = kImageWidth;
        msg.encoding = sensor_msgs::image_encodings::MONO8;
        msg.is_bigendian = false;
        msg.step = kImageWidth;
        msg.data.assign(kImageWidth * kImageHeight, 0);
        return msg;
    }

    sensor_msgs::msg::CameraInfo makeCameraInfo() {
        // PinholeCameraModel requires non-empty calibration fields before it is
        // considered initialized by cam_feature.
        sensor_msgs::msg::CameraInfo msg;
        msg.header.stamp = test_node_->get_clock()->now();
        msg.header.frame_id = "camera";
        msg.height = kImageHeight;
        msg.width = kImageWidth;
        msg.distortion_model = "plumb_bob";
        msg.d = {0.0, 0.0, 0.0, 0.0, 0.0};
        msg.k = {100.0, 0.0, 32.0, 0.0, 100.0, 24.0, 0.0, 0.0, 1.0};
        msg.r = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
        msg.p = {100.0, 0.0, 32.0, 0.0, 0.0, 100.0,
                 24.0,  0.0, 0.0,  0.0, 1.0, 0.0};
        return msg;
    }

    void forceDiagnosticsUpdate() {
        // diagnostic_updater normally publishes on its own timer. For tests,
        // force_update() makes publication deterministic and fast.
        cam_feature_->get_diagnostic_updater()->force_update();
        executor_.spin_some();
    }

    std::optional<diagnostic_msgs::msg::DiagnosticStatus> getCamFeatureStatus()
        const {
        if (!last_diagnostics_) {
            return std::nullopt;
        }

        const auto& statuses = last_diagnostics_->status;
        const auto it = std::find_if(
            statuses.begin(), statuses.end(),
            [](const diagnostic_msgs::msg::DiagnosticStatus& status) {
                return status.name.find(kCamFeatureStatusName) !=
                       std::string::npos;
            });

        if (it == statuses.end()) {
            return std::nullopt;
        }

        return *it;
    }

    std::optional<std::string> getValue(
        const diagnostic_msgs::msg::DiagnosticStatus& status,
        const std::string& key) const {
        const auto it =
            std::find_if(status.values.begin(), status.values.end(),
                         [&key](const diagnostic_msgs::msg::KeyValue& value) {
                             return value.key == key;
                         });

        if (it == status.values.end()) {
            return std::nullopt;
        }

        return it->value;
    }

    bool waitForCamFeatureStatus(
        const std::function<
            bool(const diagnostic_msgs::msg::DiagnosticStatus&)>& predicate,
        const std::chrono::milliseconds timeout) {
        // ROS discovery, lifecycle autostart, and async map updates are all
        // eventually consistent, so tests poll the published status.
        return waitUntil(
            [&]() {
                forceDiagnosticsUpdate();
                const auto status = getCamFeatureStatus();
                return status && predicate(*status);
            },
            timeout);
    }

    void publishImageUntilProcessed(
        const std::function<
            bool(const diagnostic_msgs::msg::DiagnosticStatus&)>& predicate) {
        // Re-publish images while spinning because discovery may not have
        // connected the publisher and subscription on the first attempt.
        ASSERT_TRUE(waitUntil(
            [&]() {
                image_pub_->publish(makeImage());
                forceDiagnosticsUpdate();
                const auto status = getCamFeatureStatus();
                return status && predicate(*status);
            },
            5s));
    }

    rclcpp::executors::SingleThreadedExecutor executor_;
    rclcpp::Node::SharedPtr test_node_;
    clover2::cam_feature::cam_feature::SharedPtr cam_feature_;
    rclcpp::Service<clover2_pose_msgs::srv::GetMap>::SharedPtr map_service_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_pub_;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        diagnostics_sub_;
    std::optional<diagnostic_msgs::msg::DiagnosticArray> last_diagnostics_;
};

TEST_F(CamFeatureDiagnosticsTest, ReportsSkippedFramesWithoutCameraInfo) {
    // Image callbacks should still update diagnostics even when the pipeline
    // skips processing because CameraInfo has not arrived yet.
    publishImageUntilProcessed(
        [this](const diagnostic_msgs::msg::DiagnosticStatus& status) {
            const auto skipped = getValue(status, "Skipped frames");
            return skipped && std::stoul(*skipped) > 0;
        });

    const auto status = getCamFeatureStatus();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->level, diagnostic_msgs::msg::DiagnosticStatus::WARN);
    EXPECT_EQ(status->message, "Waiting for Camera Info");

    const auto image_age = getValue(*status, "Last image age, sec");
    ASSERT_TRUE(image_age);
    EXPECT_NE(*image_age, "never");

    const auto camera_info_age = getValue(*status, "Last camera info age, sec");
    ASSERT_TRUE(camera_info_age);
    EXPECT_EQ(*camera_info_age, "never");
}

TEST_F(CamFeatureDiagnosticsTest, ReportsImageProcessingDiagnostics) {
    // The map client updates asynchronously after configure(), so wait for the
    // fake service response to be reflected in diagnostics before processing.
    ASSERT_TRUE(waitForCamFeatureStatus(
        [this](const diagnostic_msgs::msg::DiagnosticStatus& status) {
            const auto map_valid = getValue(status, "Map valid");
            return map_valid && *map_valid == "true";
        },
        5s));

    camera_info_pub_->publish(makeCameraInfo());

    // With camera calibration available, the same black image should pass
    // through the processing path and increment processed-frame diagnostics.
    publishImageUntilProcessed(
        [this](const diagnostic_msgs::msg::DiagnosticStatus& status) {
            const auto processed = getValue(status, "Processed frames");
            return processed && std::stoul(*processed) > 0;
        });

    const auto status = getCamFeatureStatus();
    ASSERT_TRUE(status);

    const auto map_valid = getValue(*status, "Map valid");
    ASSERT_TRUE(map_valid);
    EXPECT_EQ(*map_valid, "true");

    const auto image_size = getValue(*status, "Last image size");
    ASSERT_TRUE(image_size);
    EXPECT_EQ(*image_size, "64x48");

    const auto image_encoding = getValue(*status, "Last image encoding");
    ASSERT_TRUE(image_encoding);
    EXPECT_EQ(*image_encoding, sensor_msgs::image_encodings::MONO8);

    const auto processing_time = getValue(*status, "Last processing time, ms");
    ASSERT_TRUE(processing_time);
    EXPECT_GE(std::stod(*processing_time), 0.0);

    EXPECT_TRUE(getValue(*status, "Last marker count"));
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    rclcpp::init(argc, argv);
    const int ret = RUN_ALL_TESTS();
    rclcpp::shutdown();
    return ret;
}
