#pragma once

#include <clover2_common/node_context.hpp>

#include <rclcpp/serialized_message.hpp>

#include <functional>
#include <memory>
#include <string>

namespace clover2_http::plugins::utils {

class universal_subscriber {
public:
    using callback =
        std::function<void(std::shared_ptr<rclcpp::SerializedMessage>)>;

    universal_subscriber(
        std::shared_ptr<clover2_common::node_context> node_context,
        const std::string& topic_name, const std::string& topic_type,
        callback cb);
    ~universal_subscriber();

    universal_subscriber(const universal_subscriber&) = delete;
    universal_subscriber& operator=(const universal_subscriber&) = delete;

private:
    std::shared_ptr<rclcpp::GenericSubscription> m_subscription;
};

}  // namespace clover2_http::plugins::utils
