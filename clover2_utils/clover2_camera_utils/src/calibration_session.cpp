#include <clover2_camera_utils/calibration/sample/orientation.hpp>
#include <clover2_camera_utils/calibration_session.hpp>

#include <opencv2/imgproc.hpp>

#include <utility>

namespace clover2_camera_utils {
namespace {

constexpr size_t k_initial_estimate_sample_count = 15;

}  // namespace

calibration_session::calibration_session(
    data::sample_selector_config sample_selector_config)
    : sample_selector(std::move(sample_selector_config)) {}

calibration_session::image_processing_result calibration_session::process_image(
    const cv::Mat& image, const bool create_debug_image) {
    image_processing_result result;
    auto detection = target->detect(image);
    auto sample = std::move(detection.sample);
    result.board_detected = sample.has_value();
    result.feedback_stage = result.board_detected
                                ? calibrate_camera::Feedback::BOARD_DETECTED
                                : calibrate_camera::Feedback::WAITING_FOR_BOARD;

    if (create_debug_image || sample) {
        cv::Mat debug_image;
        cv::cvtColor(image, debug_image, cv::COLOR_GRAY2BGR);
        target->draw_debug(debug_image, detection.debug);
        result.debug_image = std::move(debug_image);
    }

    if (!sample) {
        return result;
    }

    std::optional<data::sample_selection_result> selection;
    if (!camera_estimate) {
        selection = sample_selector.try_add(std::move(*sample));
        if (selection->accepted && sample_selector.accepted_count() >=
                                       k_initial_estimate_sample_count) {
            camera_estimate =
                algorithm->calibrate(sample_selector.observations());
            if (!camera_estimate) {
                result.initialization_event =
                    calibration_initialization_event::initial_estimate_failed;
            } else {
                auto corrected_features = sample_selector.features();
                const auto& observations = sample_selector.observations();
                bool tilt_ready =
                    corrected_features.size() == observations.size();
                for (size_t index = 0; index < observations.size(); ++index) {
                    const auto undistorted_points = algorithm->undistort_points(
                        observations[index].image_points,
                        camera_estimate->camera_info);
                    if (!undistorted_points) {
                        tilt_ready = false;
                        break;
                    }
                    const auto orientation =
                        calibration::sample::orientation_calculator{}.compute(
                            observations[index].object_points,
                            *undistorted_points, target->full_object_points());
                    if (!orientation) {
                        tilt_ready = false;
                        break;
                    }
                    corrected_features[index].tilt_x = orientation->tilt_x;
                    corrected_features[index].tilt_y = orientation->tilt_y;
                }
                if (!tilt_ready || !sample_selector.replace_features(
                                       std::move(corrected_features))) {
                    camera_estimate.reset();
                    result.initialization_event =
                        calibration_initialization_event::
                            tilt_correction_failed;
                } else {
                    sample_selector.set_tilt_selection_enabled(true);
                    selection->progress = sample_selector.progress();
                    result.initialization_event =
                        calibration_initialization_event::
                            tilt_selection_enabled;
                }
            }
        }
    } else {
        selection = add_corrected_sample(*sample);
    }

    const bool data_sufficient =
        selection && selection->progress.data_sufficient && camera_estimate;
    if (data_sufficient) {
        result.feedback_stage = calibrate_camera::Feedback::DATA_SUFFICIENT;
    } else if (selection && selection->accepted) {
        result.feedback_stage = calibrate_camera::Feedback::SAMPLE_ACCEPTED;
    }
    return result;
}

std::optional<calibration::algorithm::result>
calibration_session::run_calibration() const {
    if (!algorithm || !camera_estimate) {
        return std::nullopt;
    }
    return algorithm->calibrate(sample_selector.observations(),
                                camera_estimate->camera_info);
}

std::optional<data::sample_selection_result>
calibration_session::add_corrected_sample(data::target_detection& detection) {
    if (!camera_estimate || !algorithm || !target) {
        return std::nullopt;
    }
    const auto undistorted_points = algorithm->undistort_points(
        detection.calibration.image_points, camera_estimate->camera_info);
    if (!undistorted_points) {
        return std::nullopt;
    }
    const auto orientation =
        calibration::sample::orientation_calculator{}.compute(
            detection.calibration.object_points, *undistorted_points,
            target->full_object_points());
    if (!orientation) {
        return std::nullopt;
    }
    detection.selection.tilt_x = orientation->tilt_x;
    detection.selection.tilt_y = orientation->tilt_y;
    return sample_selector.try_add(std::move(detection));
}

}  // namespace clover2_camera_utils
