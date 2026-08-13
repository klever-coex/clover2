#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <memory>
#include <string>

namespace clover2::map::diagnostics {

class map_client_task : public diagnostic_updater::DiagnosticTask {
public:
    explicit map_client_task(const std::string& name = "/localization/map/client");

    void set_name_getter(const std::function<std::string()>& getter) {
        m_name_getter = getter;
    }

    void set_frame_id_getter(const std::function<std::string()>& getter) {
        m_frame_id_getter = getter;
    }

    void set_marker_count_getter(const std::function<int()>& getter) {
        m_marker_count_getter = getter;
    }

    void set_map_valid_getter(const std::function<bool()>& getter) {
        m_map_valid_getter = getter;
    }

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    std::function<std::string()> m_name_getter{nullptr};
    std::function<std::string()> m_frame_id_getter{nullptr};
    std::function<int()> m_marker_count_getter{nullptr};
    std::function<bool()> m_map_valid_getter{nullptr};
};

}  // namespace clover2::map::diagnostics
