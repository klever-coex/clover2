#include <clover2/localization/queue/frame_queue.hpp>

namespace clover2::localization::queue {

void frame_queue::push(data::frame_data frame) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(frame));
    }
    m_cond.notify_one();
}

std::optional<data::frame_data> frame_queue::pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this] { return !m_queue.empty(); });
    data::frame_data frame = std::move(m_queue.front());
    m_queue.pop();
    return frame;
}

std::optional<data::frame_data> frame_queue::try_pop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return std::nullopt;
    }
    data::frame_data frame = std::move(m_queue.front());
    m_queue.pop();
    return frame;
}

bool frame_queue::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

size_t frame_queue::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

}  // namespace clover2::localization::queue
