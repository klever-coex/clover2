#pragma once

#include <clover2_common/diagnostics/client.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_notification/provider/base.hpp>

#include <string>
#include <unordered_map>

namespace clover2_notification::provider {

class diagnostics final : public base {
public:
    static constexpr const char* name = "diagnostics";

    diagnostics() = default;
    ~diagnostics() override = default;

    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    callback_type callback) override;
    void cleanup() override;

private:
    using status_type = clover2_common::diagnostics::client::status_type;
    using status_map = std::unordered_map<std::string, status_type>;

    void diagnostics_callback(const status_type& status);

    std::shared_ptr<clover2_common::node_context> m_node_context;
    callback_type m_callback;
    clover2_common::diagnostics::client m_client;
    status_map m_previous;
    std::optional<rclcpp::Logger> m_logger;
};

}  // namespace clover2_notification::provider
