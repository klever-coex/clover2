#pragma once

// ROS2
#include <nlohmann/json.hpp>
#include <rclcpp/qos.hpp>

// STL
#include <string>

namespace clover2_http::plugins::data {

struct qos {
    std::string history;
    int64_t depth = 0;
    std::string reliability;
    std::string durability;
    std::string liveliness;
    int64_t deadline_ns = 0;
    int64_t lifespan_ns = 0;
    int64_t liveliness_lease_duration_ns = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(qos, history, depth, reliability, durability,
                                   liveliness, deadline_ns, lifespan_ns,
                                   liveliness_lease_duration_ns)

qos to_qos(const rclcpp::QoS& profile);

}  // namespace clover2_http::plugins::data
