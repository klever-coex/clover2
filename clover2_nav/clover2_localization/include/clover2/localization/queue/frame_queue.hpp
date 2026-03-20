#pragma once

#include <clover2/localization/data/frame_data.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <optional>

namespace clover2::localization::queue {

class frame_queue {
public:
    void push(data::frame_data frame);
    std::optional<data::frame_data> pop();
    std::optional<data::frame_data> try_pop();
    bool empty() const;
    size_t size() const;

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<data::frame_data> m_queue;
};

}  // namespace clover2::localization::queue
