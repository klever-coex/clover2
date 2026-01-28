#pragma once

#include <Eigen/Geometry>
#include <rclcpp/logger.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace rclcpp::node_interfaces {
class NodeBaseInterface;
class NodeLoggingInterface;
class NodeParametersInterface;
class NodeTimersInterface;
}  // namespace rclcpp::node_interfaces

namespace clover2::aruco {
class map_client;
}  // namespace clover2::aruco

namespace clover2::aruco::optimizer {

struct marker {
    int id;
    Eigen::Affine3d transform;
    Eigen::Matrix<double, 6, 6> cov;
};

struct context {
    std::shared_ptr<rclcpp::node_interfaces::NodeBaseInterface> node_base;
    std::shared_ptr<rclcpp::node_interfaces::NodeLoggingInterface> node_logging;
    std::shared_ptr<rclcpp::node_interfaces::NodeParametersInterface>
        node_parameters;
    std::shared_ptr<rclcpp::node_interfaces::NodeTimersInterface> node_timers;

    std::shared_ptr<clover2::aruco::map_client> map_client;
};

class base_optimizer {
public:
    using data_ready_callback_t =
        std::function<void(const marker&, std::chrono::nanoseconds)>;

    base_optimizer(const clover2::aruco::optimizer::context& ctx);
    virtual ~base_optimizer() = default;

    virtual void optimize() = 0;

    virtual void push_measurement(std::string& source_frame,
                                  std::chrono::nanoseconds timestamp,
                                  std::vector<marker>& measurement) = 0;
    virtual void clear_measurements() = 0;

    void set_data_ready_callback(data_ready_callback_t callback);

protected:
    void notify_data_ready(const marker& pose,
                           std::chrono::nanoseconds timestamp);

    context m_ctx;
    rclcpp::Logger m_logger;

private:
    data_ready_callback_t m_data_ready_callback;
};

}  // namespace clover2::aruco::optimizer
