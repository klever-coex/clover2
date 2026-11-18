#include <clover2_camera_utils/calibration/sample/orientation.hpp>
#include <clover2_camera_utils/calibration_session.hpp>

#include <opencv2/imgproc.hpp>

#include <utility>

namespace clover2_camera_utils {
namespace {

constexpr size_t k_initial_estimate_sample_count = 15;

}  // namespace

calibration_session::calibration_session(
    data::sample_selector_config sample_selector_config,
    std::shared_ptr<const calibration::target::base> target,
    std::unique_ptr<calibration::algorithm::base> algorithm)
    : m_target(std::move(target))
    , m_algorithm(std::move(algorithm))
    , m_sample_selector(std::move(sample_selector_config)) {
    m_sample_selector.set_tilt_selection_enabled(false);
}

calibration_session::image_processing_result calibration_session::process_image(
    const cv::Mat& image, const bool create_debug_image) {
    image_processing_result result;
    auto detection = m_target->detect(image);
    auto sample = std::move(detection.sample);
    result.board_detected = sample.has_value();
    result.feedback_stage = result.board_detected
                                ? calibrate_camera::Feedback::BOARD_DETECTED
                                : calibrate_camera::Feedback::WAITING_FOR_BOARD;

    if (create_debug_image || sample) {
        cv::Mat debug_image;
        cv::cvtColor(image, debug_image, cv::COLOR_GRAY2BGR);
        m_target->draw_debug(debug_image, detection.debug);
        result.debug_image = std::move(debug_image);
    }

    if (!sample) {
        return result;
    }

    std::optional<data::sample_selection_result> selection;
    if (!m_camera_estimate) {
        selection = m_sample_selector.try_add(std::move(*sample));
        if (selection->accepted && m_sample_selector.accepted_count() >=
                                       k_initial_estimate_sample_count) {
            result.initialization_event = initialize_camera_estimate();
            if (result.initialization_event ==
                calibration_initialization_event::tilt_selection_enabled) {
                selection->progress = m_sample_selector.progress();
            }
        }
    } else {
        selection = add_corrected_sample(*sample);
    }

    const bool data_sufficient =
        selection && selection->progress.data_sufficient && m_camera_estimate;
    if (data_sufficient) {
        result.feedback_stage = calibrate_camera::Feedback::DATA_SUFFICIENT;
    } else if (selection && selection->accepted) {
        result.feedback_stage = calibrate_camera::Feedback::SAMPLE_ACCEPTED;
    }
    return result;
}

bool calibration_session::ready_to_calibrate() const {
    return m_algorithm && m_camera_estimate &&
           m_sample_selector.progress().data_sufficient;
}

std::optional<calibration::algorithm::result>
calibration_session::run_calibration() const {
    if (!ready_to_calibrate()) {
        return std::nullopt;
    }
    return m_algorithm->calibrate(m_sample_selector.observations(),
                                  m_camera_estimate->camera_info);
}

std::optional<data::sample_selection_result>
calibration_session::add_corrected_sample(data::target_detection& detection) {
    const auto orientation =
        calculate_corrected_orientation(detection.calibration);
    if (!orientation) {
        return std::nullopt;
    }
    detection.selection.tilt_x = orientation->tilt_x;
    detection.selection.tilt_y = orientation->tilt_y;
    return m_sample_selector.try_add(std::move(detection));
}

calibration_session::calibration_initialization_event
calibration_session::initialize_camera_estimate() {
    m_camera_estimate = m_algorithm->calibrate(m_sample_selector.observations());
    if (!m_camera_estimate) {
        return calibration_initialization_event::initial_estimate_failed;
    }
    if (!correct_collected_sample_tilts()) {
        m_camera_estimate.reset();
        return calibration_initialization_event::tilt_correction_failed;
    }
    m_sample_selector.set_tilt_selection_enabled(true);
    return calibration_initialization_event::tilt_selection_enabled;
}

bool calibration_session::correct_collected_sample_tilts() {
    auto corrected_features = m_sample_selector.features();
    const auto& observations = m_sample_selector.observations();
    if (corrected_features.size() != observations.size()) {
        return false;
    }
    for (size_t index = 0; index < observations.size(); ++index) {
        const auto orientation =
            calculate_corrected_orientation(observations[index]);
        if (!orientation) {
            return false;
        }
        corrected_features[index].tilt_x = orientation->tilt_x;
        corrected_features[index].tilt_y = orientation->tilt_y;
    }
    return m_sample_selector.replace_features(std::move(corrected_features));
}

std::optional<data::board_orientation>
calibration_session::calculate_corrected_orientation(
    const data::observation& observation) const {
    if (!m_camera_estimate || !m_algorithm || !m_target) {
        return std::nullopt;
    }
    const auto undistorted_points = m_algorithm->undistort_points(
        observation.image_points, m_camera_estimate->camera_info);
    if (!undistorted_points) {
        return std::nullopt;
    }
    return calibration::sample::orientation_calculator{}.compute(
        observation.object_points, *undistorted_points,
        m_target->full_object_points());
}

}  // namespace clover2_camera_utils
