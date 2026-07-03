#include <clover2/aruco/tracker.hpp>
#include <clover2_pose_msgs/msg/marker_array.hpp>
#include <clover2_pose_msgs/srv/get_map.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>

namespace {

using namespace std::chrono_literals;

constexpr const char* kArucoTrackerStatusName = "Aruco tracker status";
constexpr uint32_t kMarkerId = 42;

class ArucoTrackerDiagnosticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_node_ =
            std::make_shared<rclcpp::Node>("aruco_tracker_diag_test");

        // tracker creates clover2::map::client during configure(), and the
        // client synchronously waits for this private service to exist.
        map_service_ =
            test_node_->create_service<clover2_pose_msgs::srv::GetMap>(
                "/tracker/get_map",
                [](const clover2_pose_msgs::srv::GetMap::Request::SharedPtr,
                   clover2_pose_msgs::srv::GetMap::Response::SharedPtr
                       response) {
                    response->map.header.frame_id = "map";
                    response->map.name = "test_map";
                    response->map.markers.push_back(makeMapMarker());
                });

        // Test the public ROS diagnostics stream instead of calling the private
        // produce_diagnostics() method directly.
        diagnostics_sub_ = test_node_->create_subscription<
            diagnostic_msgs::msg::DiagnosticArray>(
            "/diagnostics", rclcpp::QoS(10),
            [this](diagnostic_msgs::msg::DiagnosticArray::ConstSharedPtr msg) {
                last_diagnostics_ = *msg;
            });

        markers_pub_ =
            test_node_->create_publisher<clover2_pose_msgs::msg::MarkerArray>(
                "/tracker/markers", rclcpp::SensorDataQoS());

        tracker_ =
            std::make_shared<clover2::aruco::tracker>(rclcpp::NodeOptions());

        // tracker autostarts from the common lifecycle base. Spinning this
        // executor drives configure(), activate(), service replies, topic
        // callbacks, and diagnostics delivery.
        executor_.add_node(test_node_);
        executor_.add_node(tracker_->get_node_base_interface());
    }

    void TearDown() override {
        executor_.remove_node(tracker_->get_node_base_interface());
        executor_.remove_node(test_node_);

        tracker_.reset();
        test_node_.reset();
        last_diagnostics_.reset();
    }

    static clover2_pose_msgs::msg::Marker makeMapMarker() {
        clover2_pose_msgs::msg::Marker marker;
        marker.id = kMarkerId;
        marker.type = clover2_pose_msgs::msg::Marker::TYPE_ARUCO;
        marker.size = 0.2;
        marker.marker_frame_id = "marker_42";
        marker.pose.pose.orientation.w = 1.0;
        return marker;
    }

    clover2_pose_msgs::msg::MarkerArray makeMarkerArray(
        const bool include_marker) {
        clover2_pose_msgs::msg::MarkerArray msg;
        msg.header.stamp = test_node_->get_clock()->now();
        msg.header.frame_id = "camera";

        if (include_marker) {
            auto marker = makeMapMarker();
            marker.pose.pose.position.z = 1.0;
            msg.markers.push_back(marker);
        }

        return msg;
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

    void forceDiagnosticsUpdate() {
        // diagnostic_updater normally publishes on its own timer. For tests,
        // force_update() makes publication deterministic and fast.
        tracker_->get_diagnostic_updater()->force_update();
        executor_.spin_some();
    }

    std::optional<diagnostic_msgs::msg::DiagnosticStatus>
    getArucoTrackerStatus() const {
        if (!last_diagnostics_) {
            return std::nullopt;
        }

        const auto& statuses = last_diagnostics_->status;
        const auto it = std::find_if(
            statuses.begin(), statuses.end(),
            [](const diagnostic_msgs::msg::DiagnosticStatus& status) {
                return status.name.find(kArucoTrackerStatusName) !=
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

    bool waitForArucoTrackerStatus(
        const std::function<
            bool(const diagnostic_msgs::msg::DiagnosticStatus&)>& predicate,
        const std::chrono::milliseconds timeout) {
        // ROS discovery, lifecycle autostart, and async map updates are all
        // eventually consistent, so tests poll the published status.
        return waitUntil(
            [&]() {
                forceDiagnosticsUpdate();
                const auto status = getArucoTrackerStatus();
                return status && predicate(*status);
            },
            timeout);
    }

    void publishMarkersUntilStatus(
        const clover2_pose_msgs::msg::MarkerArray& msg,
        const std::function<
            bool(const diagnostic_msgs::msg::DiagnosticStatus&)>& predicate) {
        // Re-publish marker arrays while spinning because discovery may not
        // have connected the publisher and subscription on the first attempt.
        ASSERT_TRUE(waitUntil(
            [&]() {
                markers_pub_->publish(msg);
                forceDiagnosticsUpdate();
                const auto status = getArucoTrackerStatus();
                return status && predicate(*status);
            },
            5s));
    }

    rclcpp::executors::SingleThreadedExecutor executor_;
    rclcpp::Node::SharedPtr test_node_;
    clover2::aruco::tracker::SharedPtr tracker_;
    rclcpp::Service<clover2_pose_msgs::srv::GetMap>::SharedPtr map_service_;
    rclcpp::Publisher<clover2_pose_msgs::msg::MarkerArray>::SharedPtr
        markers_pub_;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        diagnostics_sub_;
    std::optional<diagnostic_msgs::msg::DiagnosticArray> last_diagnostics_;
};

TEST_F(ArucoTrackerDiagnosticsTest, ReportsWaitingForMarkers) {
    ASSERT_TRUE(waitForArucoTrackerStatus(
        [](const diagnostic_msgs::msg::DiagnosticStatus& status) {
            return status.message == "Waiting for markers";
        },
        5s));

    const auto status = getArucoTrackerStatus();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->level, diagnostic_msgs::msg::DiagnosticStatus::WARN);

    const auto markers_age = getValue(*status, "Last markers age, sec");
    ASSERT_TRUE(markers_age);
    EXPECT_EQ(*markers_age, "never");

    const auto pose_age = getValue(*status, "Last pose publish age, sec");
    ASSERT_TRUE(pose_age);
    EXPECT_EQ(*pose_age, "never");
}

TEST_F(ArucoTrackerDiagnosticsTest, ReportsEmptyMarkerArray) {
    publishMarkersUntilStatus(
        makeMarkerArray(false),
        [this](const diagnostic_msgs::msg::DiagnosticStatus& status) {
            const auto skipped = getValue(status, "Skipped marker arrays");
            return status.message == "No visible markers" && skipped &&
                   std::stoul(*skipped) > 0;
        });

    const auto status = getArucoTrackerStatus();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->level, diagnostic_msgs::msg::DiagnosticStatus::WARN);

    const auto marker_count = getValue(*status, "Last marker count");
    ASSERT_TRUE(marker_count);
    EXPECT_EQ(*marker_count, "0");

    const auto markers_age = getValue(*status, "Last markers age, sec");
    ASSERT_TRUE(markers_age);
    EXPECT_NE(*markers_age, "never");
}

TEST_F(ArucoTrackerDiagnosticsTest, ReportsMissingRequiredTf) {
    publishMarkersUntilStatus(
        makeMarkerArray(true),
        [this](const diagnostic_msgs::msg::DiagnosticStatus& status) {
            const auto tf_ok = getValue(status, "Camera/base TF ok");
            return status.message == "Required TF transform failed" && tf_ok &&
                   *tf_ok == "false";
        });

    const auto status = getArucoTrackerStatus();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->level, diagnostic_msgs::msg::DiagnosticStatus::ERROR);

    const auto marker_count = getValue(*status, "Last marker count");
    ASSERT_TRUE(marker_count);
    EXPECT_EQ(*marker_count, "1");

    const auto skipped = getValue(*status, "Skipped marker arrays");
    ASSERT_TRUE(skipped);
    EXPECT_GT(std::stoul(*skipped), 0U);

    const auto pose_age = getValue(*status, "Last pose publish age, sec");
    ASSERT_TRUE(pose_age);
    EXPECT_EQ(*pose_age, "never");
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    rclcpp::init(argc, argv);
    const int ret = RUN_ALL_TESTS();
    rclcpp::shutdown();
    return ret;
}
