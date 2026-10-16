#include <clover2_display/client.hpp>
#include <clover2_display_msgs/srv/get_driver_info.hpp>
#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

using namespace std::chrono_literals;

namespace {

using GetDriverInfo = clover2_display_msgs::srv::GetDriverInfo;

class DisplayClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_client_node = std::make_shared<rclcpp::Node>("display_client_test");
        m_driver_node = std::make_shared<rclcpp::Node>("display_driver_mock");

        m_executor.add_node(m_client_node);
        m_executor.add_node(m_driver_node);
    }

    void TearDown() override {
        m_executor.remove_node(m_client_node);
        m_executor.remove_node(m_driver_node);

        m_info_service.reset();
        m_image_sub.reset();
        m_client_node.reset();
        m_driver_node.reset();
    }

    void create_info_service() {
        m_info_service = m_driver_node->create_service<GetDriverInfo>(
            "/display/get_driver_info",
            [](const GetDriverInfo::Request::SharedPtr /* request */,
               GetDriverInfo::Response::SharedPtr response) {
                response->success = true;
                response->message = "ok";
                response->width = 128;
                response->height = 64;
                response->max_fps = 10.0;
                response->supported_encodings = {"mono8"};
            });
    }

    void create_image_subscription() {
        m_image_sub = m_driver_node->create_subscription<sensor_msgs::msg::Image>(
            "/display/image", rclcpp::SystemDefaultsQoS(),
            [this](const sensor_msgs::msg::Image& msg) {
                m_last_image = msg;
                m_image_received = true;
            });
    }

    bool spin_until(const std::function<bool()>& predicate,
                    std::chrono::milliseconds timeout) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            m_executor.spin_some();
            if (predicate()) {
                return true;
            }
            std::this_thread::sleep_for(10ms);
        }
        return predicate();
    }

    std::shared_ptr<rclcpp::Node> m_client_node;
    std::shared_ptr<rclcpp::Node> m_driver_node;
    rclcpp::executors::SingleThreadedExecutor m_executor;

    rclcpp::Service<GetDriverInfo>::SharedPtr m_info_service;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;

    bool m_image_received{false};
    sensor_msgs::msg::Image m_last_image;
};

}  // namespace

TEST_F(DisplayClientTest, GetsDriverInfo) {
    create_info_service();

    clover2_display::client client(m_client_node, "/display");

    ASSERT_TRUE(spin_until([&client]() { return client.valid(); }, 1s));

    EXPECT_EQ(client.get_width(), 128u);
    EXPECT_EQ(client.get_height(), 64u);
    EXPECT_DOUBLE_EQ(client.get_max_fps(), 10.0);

    const auto& encodings = client.get_supported_encodings();
    ASSERT_EQ(encodings.size(), 1u);
    EXPECT_EQ(encodings[0], "mono8");

    const auto& info = client.get_info();
    EXPECT_EQ(info.width, 128u);
    EXPECT_EQ(info.height, 64u);
    EXPECT_DOUBLE_EQ(info.max_fps, 10.0);
}

TEST_F(DisplayClientTest, PublishesImage) {
    create_info_service();
    create_image_subscription();

    clover2_display::client client(m_client_node, "/display");

    ASSERT_TRUE(spin_until([&client]() { return client.valid(); }, 1s));
    ASSERT_TRUE(spin_until(
        [&client]() { return client.get_publisher()->get_subscription_count() > 0; },
        1s));

    sensor_msgs::msg::Image image;
    image.width = 128;
    image.height = 64;
    image.encoding = "mono8";
    image.step = image.width;
    image.data.assign(image.height * image.step, 255);

    client.send_image(image);

    ASSERT_TRUE(spin_until([this]() { return m_image_received; }, 1s));

    EXPECT_EQ(m_last_image.width, 128u);
    EXPECT_EQ(m_last_image.height, 64u);
    EXPECT_EQ(m_last_image.encoding, "mono8");
    EXPECT_EQ(m_last_image.step, 128u);
    EXPECT_EQ(m_last_image.data.size(), 128u * 64u);
    EXPECT_EQ(m_last_image.data[0], 255);
}
