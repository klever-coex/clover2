/**
 * @file sample_selection_result.hpp
 * @brief Provides a sample selection result.
 */

#pragma once

#include <clover2_camera_utils/data/coverage_progress.hpp>

namespace clover2_camera_utils::data {

/** @brief Indicates the reason for sample rejection. */
enum class sample_rejection_reason {
    none,                 ///< Sample accepted.
    invalid_observation,  ///< Observation does not allow sample feature
                          ///< computation.
    duplicate,  ///< Sample duplicates a previously accepted observation.
};

/** @brief Stores the result of an attempt to add a sample to the calibration
 * dataset. */
struct sample_selection_result {
    bool accepted{false};  ///< Whether the sample was accepted.
    sample_rejection_reason rejection_reason{
        sample_rejection_reason::none};  ///< Sample rejection reason.
    coverage_progress
        progress;  ///< Coverage progress after processing the sample.
};

}  // namespace clover2_camera_utils::data
