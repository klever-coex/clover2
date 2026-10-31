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

template <typename T>
class telemetry_interface
    : public std::enable_shared_from_this<telemetry_interface<T>> {
public:
    using update_callback = std::function<void(const T&)>;
    using callbacks = clover2_common::util::shared_callback<void(const T&)>;
    using token = typename callbacks::token;

    explicit telemetry_interface(capability::base_interface::shared_ptr base)
        : m_clock(base->clock()) {}

    virtual ~telemetry_interface() = default;

    /// @brief Subscribe a callback executed on every new snapshot.
    token on_update(update_callback callback) {
        return m_callbacks.add(this->shared_from_this(), std::move(callback));
    }

    /// @brief Get a copy of the latest snapshot.
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

    /// @brief Check that the latest snapshot is still fresh.
    template <typename rep_t, typename period_t>
    bool last_valid(std::chrono::duration<rep_t, period_t> max_delay =
                        std::chrono::milliseconds(500)) const {
        const std::lock_guard<std::mutex> lock(m_data_mutex);
        return m_has_data && (m_clock->now() - m_last_time) < max_delay;
    }

protected:
    /// @brief Publish a new snapshot into the interface.
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
