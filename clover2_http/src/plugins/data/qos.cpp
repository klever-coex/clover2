// clover2
#include <clover2_http/plugins/data/qos.hpp>

// STL
#include <string>
#include <unordered_map>

namespace clover2_http::plugins::data {

namespace {

std::string to_string(rclcpp::HistoryPolicy policy) {
    static const std::unordered_map<rclcpp::HistoryPolicy, std::string> names =
        {
            {rclcpp::HistoryPolicy::KeepLast, "keep_last"},
            {rclcpp::HistoryPolicy::KeepAll, "keep_all"},
            {rclcpp::HistoryPolicy::Unknown, "unknown"},
        };

    const auto found = names.find(policy);
    return found != names.cend() ? found->second : "system_default";
}

std::string to_string(rclcpp::ReliabilityPolicy policy) {
    static const std::unordered_map<rclcpp::ReliabilityPolicy, std::string>
        names = {
            {rclcpp::ReliabilityPolicy::BestEffort, "best_effort"},
            {rclcpp::ReliabilityPolicy::Reliable, "reliable"},
            {rclcpp::ReliabilityPolicy::BestAvailable, "best_available"},
            {rclcpp::ReliabilityPolicy::Unknown, "unknown"},
        };

    const auto found = names.find(policy);
    return found != names.cend() ? found->second : "system_default";
}

std::string to_string(rclcpp::DurabilityPolicy policy) {
    static const std::unordered_map<rclcpp::DurabilityPolicy, std::string>
        names = {
            {rclcpp::DurabilityPolicy::Volatile, "volatile"},
            {rclcpp::DurabilityPolicy::TransientLocal, "transient_local"},
            {rclcpp::DurabilityPolicy::BestAvailable, "best_available"},
            {rclcpp::DurabilityPolicy::Unknown, "unknown"},
        };

    const auto found = names.find(policy);
    return found != names.cend() ? found->second : "system_default";
}

std::string to_string(rclcpp::LivelinessPolicy policy) {
    static const std::unordered_map<rclcpp::LivelinessPolicy, std::string>
        names = {
            {rclcpp::LivelinessPolicy::Automatic, "automatic"},
            {rclcpp::LivelinessPolicy::ManualByTopic, "manual_by_topic"},
            {rclcpp::LivelinessPolicy::BestAvailable, "best_available"},
            {rclcpp::LivelinessPolicy::Unknown, "unknown"},
        };

    const auto found = names.find(policy);
    return found != names.cend() ? found->second : "system_default";
}

}  // namespace

qos to_qos(const rclcpp::QoS& profile) {
    qos result;

    result.history = to_string(profile.history());
    result.depth = static_cast<int64_t>(profile.depth());
    result.reliability = to_string(profile.reliability());
    result.durability = to_string(profile.durability());
    result.liveliness = to_string(profile.liveliness());
    result.deadline_ns = profile.deadline().nanoseconds();
    result.lifespan_ns = profile.lifespan().nanoseconds();
    result.liveliness_lease_duration_ns =
        profile.liveliness_lease_duration().nanoseconds();

    return result;
}

}  // namespace clover2_http::plugins::data
