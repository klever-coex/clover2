#pragma once

#include <atomic>
#include <unordered_map>

namespace clover2::localization::graph {

class id_manager {
public:
    int32_t next_pose_id() { return m_next_pose_id++; }
    int32_t next_landmark_id() { return m_next_landmark_id++; }

    void register_marker(int32_t marker_id, int32_t vertex_id) {
        m_marker_to_vertex[marker_id] = vertex_id;
    }

    int32_t get_vertex_for_marker(int32_t marker_id) const {
        auto it = m_marker_to_vertex.find(marker_id);
        return it != m_marker_to_vertex.end() ? it->second : -1;
    }

private:
    std::atomic<int32_t> m_next_pose_id{0};
    std::atomic<int32_t> m_next_landmark_id{10000};
    std::unordered_map<int32_t, int32_t> m_marker_to_vertex;
};

}  // namespace clover2::localization::graph
