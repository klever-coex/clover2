#pragma once

#include <deque>
#include <mutex>

namespace clover2::common::util {

template <typename ValueT, typename TimeT = double>
class time_buffer {
public:
    struct item {
        TimeT timestamp;
        ValueT data;
    };

    using Container = std::deque<item>;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    explicit time_buffer(const TimeT& history_sec)
        : m_history_sec(history_sec) {}
    virtual ~time_buffer() {}

    time_buffer(const time_buffer& other) {
        std::lock_guard<std::recursive_mutex> lock(other.m_mutex);
        m_history_sec = other.m_history_sec;
        m_buffer = other.m_buffer;
    }

    time_buffer& operator=(const time_buffer& other) {
        if (this == &other) {
            return *this;
        }

        std::scoped_lock lock(m_mutex, other.m_mutex);
        m_history_sec = other.m_history_sec;
        m_buffer = other.m_buffer;
        return *this;
    }

    void add(const TimeT& timestamp, const ValueT& data,
             bool auto_prune = true) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_buffer.push_back({timestamp, data});

        if (auto_prune) {
            prune(timestamp);
        }
    }

    void prune(const TimeT& current_time) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        while (!m_buffer.empty() &&
               m_buffer.front().timestamp < current_time - m_history_sec) {
            m_buffer.pop_front();
        }
    }

    iterator begin() { return m_buffer.begin(); }
    iterator end() { return m_buffer.end(); }

    const_iterator begin() const { return m_buffer.begin(); }
    const_iterator end() const { return m_buffer.end(); }

    const_iterator cbegin() const { return m_buffer.cbegin(); }
    const_iterator cend() const { return m_buffer.cend(); }

    size_t size() const {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return m_buffer.size();
    }

    bool empty() const {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return m_buffer.empty();
    }

    void clear() {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_buffer.clear();
    }

    TimeT windowSize() const { return m_history_sec; }

private:
    TimeT m_history_sec;
    std::deque<item> m_buffer;
    mutable std::recursive_mutex m_mutex;
};

}  // namespace clover2::common::util
