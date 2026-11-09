#pragma once

// STL
#include <cstdint>
#include <functional>
#include <utility>

namespace clover2_fcu::utils {

/// Value handle over a publish closure, by ergonomics like a ROS
/// publisher. The sensor_id is a backend detail: the backend resolves the
/// slot at handle creation and bakes it into the closure; the id stays
/// observable here for logs. A default-constructed handle has no slot
/// (operator bool == false); a handle whose owning provider died turns
/// into a quiet no-op via the weak closure. Publish from any thread —
/// all mutable state belongs to the provider.
template <typename T>
class sensor_publisher {
public:
    using publish_fn = std::function<void(const T&)>;

    sensor_publisher() = default;

    explicit sensor_publisher(uint8_t id, publish_fn publish)
        : m_id(id)
        , m_publish(std::move(publish)) {}

    /// Whether the backend granted a slot for this sensor.
    explicit operator bool() const noexcept {
        return static_cast<bool>(m_publish);
    }

    /// Backend-assigned id of the sensor slot (for logs).
    uint8_t id() const noexcept { return m_id; }

    /// Dispatch one sample; a no-op on an empty handle.
    void publish(const T& sample) const {
        if (m_publish) {
            m_publish(sample);
        }
    }

private:
    uint8_t m_id{0};
    publish_fn m_publish;
};

}  // namespace clover2_fcu::utils
