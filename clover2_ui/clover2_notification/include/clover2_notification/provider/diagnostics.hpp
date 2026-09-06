/**
 * @file diagnostics.hpp
 * @brief Provides a diagnostics-based notification provider.
 */

#pragma once

// clover2
#include <clover2_common/diagnostics/client.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_notification/provider/base.hpp>

// STL
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_notification::provider {

/**
 * @class diagnostics
 * @brief Notification provider that converts ROS 2 diagnostic status changes to
 * events.
 *
 * The provider subscribes to the configured diagnostics topic, tracks previous
 * statuses, ignores configured diagnostic names, and emits events only when a
 * non-OK diagnostic status changes.
 */
class diagnostics final : public base {
public:
    /** @brief Provider name used by the provider factory and configuration. */
    static constexpr const char* name = "diagnostics";

    /** @brief Construct a diagnostics provider. */
    diagnostics() = default;

    /** @brief Destroy a diagnostics provider. */
    ~diagnostics() override = default;

    /**
     * @brief Initialize diagnostics subscription and event callback.
     *
     * Declares and reads diagnostics provider parameters, then subscribes to
     * the configured diagnostics topic.
     *
     * @param node_context Shared node context used to access ROS 2 interfaces.
     * @param callback Callback invoked for generated notification events.
     *
     * @throws std::invalid_argument if @p node_context is null or @p callback
     * is empty.
     */
    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    callback_type callback) override;

    /** @brief Unsubscribe from diagnostics and reset provider state. */
    void cleanup() override;

private:
    using message_type = clover2_common::diagnostics::client::message_type;
    using status_type = clover2_common::diagnostics::client::status_type;
    using status_map = std::unordered_map<std::string, status_type>;

    /**
     * @brief Handle a diagnostic message update.
     *
     * @param msg Diagnostic message received from the diagnostics client.
     */
    void diagnostics_callback(const message_type& msg);

    /**
     * @brief Process a single diagnostic status.
     *
     * @param status Diagnostic status from a diagnostics message.
     */
    void process_status(const status_type& status);

    /**
     * @brief Check whether a diagnostic status should be ignored by name.
     *
     * @param status Diagnostic status to check.
     * @return true if the status matches an ignored name pattern, false
     * otherwise.
     */
    bool is_ignored(const status_type& status) const;

    std::shared_ptr<clover2_common::node_context> m_node_context;
    callback_type m_callback;
    std::shared_ptr<clover2_common::diagnostics::client> m_client;
    status_map m_previous;
    std::vector<std::string> m_ignore_name_patterns;
    std::optional<rclcpp::Logger> m_logger;
};

}  // namespace clover2_notification::provider
