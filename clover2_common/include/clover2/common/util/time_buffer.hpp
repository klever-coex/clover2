#pragma once

// STL
#include <deque>
#include <mutex>

namespace clover2::common::util {

template <typename ValueT, typename TimeT = double>
class time_buffer {
public:
    using item_type = std::pair<TimeT, ValueT>;
    using iterator = typename std::deque<item_type>::iterator;
    using const_iterator = typename std::deque<item_type>::const_iterator;

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
               m_buffer.front().first < current_time - m_history_sec) {
            m_buffer.pop_front();
        }
    }

    std::deque<item_type>& buffer() { return m_buffer; }

    item_type& front() { return m_buffer.front(); }

    const item_type& front() const { return m_buffer.front(); }

    item_type& back() { return m_buffer.back(); }

    const item_type& back() const { return m_buffer.back(); }

    size_t size() const { return m_buffer.size(); }

    bool empty() const { return m_buffer.empty(); }

    void clear() { m_buffer.clear(); }

    TimeT windowSize() const { return m_history_sec; }

private:
    TimeT m_history_sec;
    std::deque<item_type> m_buffer;
    mutable std::recursive_mutex m_mutex;
};

}  // namespace clover2::common::util
