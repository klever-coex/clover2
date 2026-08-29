#pragma once

// ROS2
#include <nlohmann/json.hpp>
#include <rclcpp/qos.hpp>

// STL
#include <cstdint>
#include <string>

namespace clover2_http::plugins::data {

namespace detail {

inline std::string to_string(rclcpp::HistoryPolicy policy) {
    switch (policy) {
        case rclcpp::HistoryPolicy::KeepLast:
            return "keep_last";
        case rclcpp::HistoryPolicy::KeepAll:
            return "keep_all";
        case rclcpp::HistoryPolicy::Unknown:
            return "unknown";
        default:
            return "system_default";
    }
}

inline std::string to_string(rclcpp::ReliabilityPolicy policy) {
    switch (policy) {
        case rclcpp::ReliabilityPolicy::BestEffort:
            return "best_effort";
        case rclcpp::ReliabilityPolicy::Reliable:
            return "reliable";
        case rclcpp::ReliabilityPolicy::BestAvailable:
            return "best_available";
        case rclcpp::ReliabilityPolicy::Unknown:
            return "unknown";
        default:
            return "system_default";
    }
}

inline std::string to_string(rclcpp::DurabilityPolicy policy) {
    switch (policy) {
        case rclcpp::DurabilityPolicy::Volatile:
            return "volatile";
        case rclcpp::DurabilityPolicy::TransientLocal:
            return "transient_local";
        case rclcpp::DurabilityPolicy::BestAvailable:
            return "best_available";
        case rclcpp::DurabilityPolicy::Unknown:
            return "unknown";
        default:
            return "system_default";
    }
}

inline std::string to_string(rclcpp::LivelinessPolicy policy) {
    switch (policy) {
        case rclcpp::LivelinessPolicy::Automatic:
            return "automatic";
        case rclcpp::LivelinessPolicy::ManualByTopic:
            return "manual_by_topic";
        case rclcpp::LivelinessPolicy::BestAvailable:
            return "best_available";
        case rclcpp::LivelinessPolicy::Unknown:
            return "unknown";
        default:
            return "system_default";
    }
}

}  // namespace detail

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

inline qos to_qos(const rclcpp::QoS& profile) {
    qos result;

    result.history = detail::to_string(profile.history());
    result.depth = static_cast<int64_t>(profile.depth());
    result.reliability = detail::to_string(profile.reliability());
    result.durability = detail::to_string(profile.durability());
    result.liveliness = detail::to_string(profile.liveliness());
    result.deadline_ns = profile.deadline().nanoseconds();
    result.lifespan_ns = profile.lifespan().nanoseconds();
    result.liveliness_lease_duration_ns =
        profile.liveliness_lease_duration().nanoseconds();

    return result;
}

}  // namespace clover2_http::plugins::data
