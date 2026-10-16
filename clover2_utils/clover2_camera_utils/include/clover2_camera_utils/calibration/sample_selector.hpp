#pragma once

#include <clover2_camera_utils/calibration/sample_features.hpp>
#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/coverage_progress.hpp>
#include <clover2_camera_utils/data/observation.hpp>
#include <clover2_camera_utils/data/sample_features.hpp>
#include <clover2_camera_utils/data/sample_selection_result.hpp>
#include <clover2_camera_utils/data/sample_selector_config.hpp>

#include <vector>

namespace clover2_camera_utils::calibration {

class sample_selector {
public:
    explicit sample_selector(data::sample_selector_config config = {});

    data::sample_selection_result try_add(data::observation observation,
                                          const data::board_config& board);

    size_t accepted_count() const noexcept;
    const std::vector<data::observation>& observations() const noexcept;
    const std::vector<data::sample_features>& features() const noexcept;
    data::coverage_progress progress() const;

private:
    data::coverage_progress compute_progress() const;
    bool is_duplicate(const data::sample_features& candidate) const;

    data::sample_selector_config m_config;
    std::vector<data::observation> m_observations;
    std::vector<data::sample_features> m_features;
};

}  // namespace clover2_camera_utils::calibration
