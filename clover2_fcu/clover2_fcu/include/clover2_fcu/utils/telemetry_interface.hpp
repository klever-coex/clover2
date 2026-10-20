#pragma once

// clover2
#include <clover2_common/util/shared_callback.hpp>
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu/common/exception.hpp>

// STL
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>

namespace clover2_fcu {

/**
 * @brief Caching base class for telemetry interfaces.
 *
 * Implements the canonical telemetry semantics on top of a data snapshot
 * type: latest value, receive time, freshness check and update callbacks.
 * Plugin implementations call @ref update_data from their transport
 * callbacks; consumers use last()/last_valid()/on_update().
 *
 * Caching base class for telemetry interfaces.
 *
 * Implements the canonical telemetry semantics on top of a data snapshot
 * type: latest value, receive time, freshness check and update callbacks.
 * Plugin implementations call @ref update_data from their transport
 * callbacks; consumers use last()/last_valid()/on_update().
 *
 * Providers are cached singletons of one client (they live as long as the
 * client), so callbacks are released with the RAII token returned by
 * on_update(); the token may safely outlive the provider. Callbacks run
 * on an executor thread of the hosting node.
 *
 * Thread-safe under any executor: the snapshot state is guarded by an
 * internal mutex and last() returns the snapshot by value (a reference
 * into data being overwritten is not salvageable); update_data() fires
 * the callbacks outside the lock, so a callback may re-enter
 * last()/on_update() re-entrantly. Subscription (shared_callback) is
 * thread-safe on its own.
 *
 * @tparam T Snapshot type (e.g. clover2_fcu::data::odometry).
 */
template <typename T>
class telemetry_interface
    : public std::enable_shared_from_this<telemetry_interface<T>> {
public:
    using update_callback = std::function<void(const T&)>;
    using callbacks = clover2_common::util::shared_callback<void(const T&)>;
    using token = typename callbacks::token;

    explicit telemetry_interface(
        capability::base_interface::shared_ptr base)
        : m_clock(base->clock()) {}

    virtual ~telemetry_interface() = default;

    /**
     * @brief Subscribe a callback executed on every new snapshot.
     * @param callback the callback function.
     * @returns RAII token; keep it while subscribed.
     *
     * The provider must be owned by a shared_ptr (it always is: handles
     * come from the client capability cache).
     */
    token on_update(update_callback callback) {
        return m_callbacks.add(this->shared_from_this(), std::move(callback));
    }

    /**
     * @brief Get a copy of the latest snapshot.
     * @returns the latest data snapshot (by value: the stored one may
     *          be overwritten concurrently).
     * @throws clover2_fcu::exception when no snapshot was received yet.
     */
    T last() const {
        const std::lock_guard<std::mutex> lock(m_data_mutex);
        if (!m_has_data) {
            throw exception("telemetry interface: no data received yet");
        }

        return m_last;
    }

    /// @brief Whether at least one snapshot was received.
    bool has_data() const {
        const std::lock_guard<std::mutex> lock(m_data_mutex);
        return m_has_data;
    }

    /// @brief Receive time of the latest snapshot.
    rclcpp::Time last_time() const {
        const std::lock_guard<std::mutex> lock(m_data_mutex);
        return m_last_time;
    }

    /**
     * @brief Check that the latest snapshot is still fresh.
     * @param max_delay maximum allowed age of the latest snapshot.
     * @return true if a snapshot exists and is younger than max_delay.
     */
    template <typename rep_t, typename period_t>
    bool last_valid(std::chrono::duration<rep_t, period_t> max_delay =
                        std::chrono::milliseconds(500)) const {
        const std::lock_guard<std::mutex> lock(m_data_mutex);
        return m_has_data && (m_clock->now() - m_last_time) < max_delay;
    }

protected:
    /**
     * @brief Publish a new snapshot into the interface.
     *
     * Called by the plugin implementation when a new transport message
     * arrives; stores the snapshot, stamps it and fires the callbacks
     * (outside the internal lock, with a copy — the stored one may be
     * overwritten by a concurrent update_data()).
     */
    void update_data(T data) {
        T stored;
        {
            const std::lock_guard<std::mutex> lock(m_data_mutex);
            m_last = std::move(data);
            m_last_time = m_clock->now();
            m_has_data = true;
            stored = m_last;
        }

        m_callbacks.invoke(stored);
    }

private:
    rclcpp::Clock::SharedPtr m_clock;

    mutable std::mutex m_data_mutex;
    T m_last{};
    rclcpp::Time m_last_time{};
    bool m_has_data{false};

    callbacks m_callbacks;
};

}  // namespace clover2_fcu
