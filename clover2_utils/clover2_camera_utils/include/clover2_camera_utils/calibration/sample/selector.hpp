/**
 * @file selector.hpp
 * @brief Provides an observation selector for camera calibration.
 */

#pragma once

#include <clover2_camera_utils/data/coverage_progress.hpp>
#include <clover2_camera_utils/data/observation.hpp>
#include <clover2_camera_utils/data/sample_features.hpp>
#include <clover2_camera_utils/data/sample_selection_result.hpp>
#include <clover2_camera_utils/data/sample_selector_config.hpp>
#include <clover2_camera_utils/data/target_detection.hpp>

#include <vector>

namespace clover2_camera_utils::calibration::sample {

/** @brief Selects valid, non-duplicate observations for calibration. */
class selector {
public:
    /**
     * @brief Constructs an observation selector.
     * @param config Sample selection configuration.
     */
    explicit selector(data::sample_selector_config config = {});

    /**
     * @brief Attempts to add a detected sample to the calibration dataset.
     * @param sample Calibration observation and target-specific features.
     * @return Sample selection result, including rejection reason and coverage
     * progress.
     */
    data::sample_selection_result try_add(data::target_detection detection);

    /**
     * @brief Returns the number of accepted observations.
     * @return Number of observations in the calibration dataset.
     */
    size_t accepted_count() const noexcept;

    /**
     * @brief Returns accepted observations.
     * @return Calibration dataset observations.
     */
    const std::vector<data::observation>& observations() const noexcept;

    /**
     * @brief Returns features of accepted samples.
     * @return Features of calibration dataset samples.
     */
    const std::vector<data::sample_features>& features() const noexcept;

    /**
     * @brief Returns the current coverage progress.
     * @return Calibration dataset coverage progress.
     */
    data::coverage_progress progress() const;

    /**
     * @brief Enables or disables tilt dimensions during sample selection.
     *
     * Disabled tilt dimensions do not participate in duplicate detection or
     * dataset-sufficiency checks. Their reported coverage is `1.0`.
     *
     * @param enabled Whether tilt dimensions are valid for selection.
     */
    void set_tilt_selection_enabled(bool enabled) noexcept;

    /**
     * @brief Replaces features for all accepted observations.
     * @param features Replacement features in observation order.
     * @return `true` when the feature count matches accepted observations.
     */
    bool replace_features(std::vector<data::sample_features> features);

private:
    data::coverage_progress compute_progress() const;
    bool is_duplicate(const data::sample_features& candidate) const;

    data::sample_selector_config m_config;
    std::vector<data::observation> m_observations;
    std::vector<data::sample_features> m_features;
    bool m_tilt_selection_enabled{true};
};

}  // namespace clover2_camera_utils::calibration::sample
