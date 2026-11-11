#pragma once

// STL
#include <cstdint>
#include <functional>
#include <utility>

namespace clover2_fcu::utils {

template <typename T>
class sensor_publisher {
public:
    using publish_fn = std::function<void(const T&)>;

    sensor_publisher() = default;

    explicit sensor_publisher(uint8_t id, publish_fn publish)
        : m_id(id)
        , m_publish(std::move(publish)) {}

    explicit operator bool() const noexcept {
        return static_cast<bool>(m_publish);
    }

    uint8_t id() const noexcept { return m_id; }

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
