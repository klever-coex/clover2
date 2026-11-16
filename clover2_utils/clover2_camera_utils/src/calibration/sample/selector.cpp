#include <clover2_camera_utils/calibration/sample/selector.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace clover2_camera_utils::calibration::sample {
namespace {

constexpr double k_progress_comparison_epsilon = 1e-6;

double distance(const data::sample_features& first,
                const data::sample_features& second,
                bool tilt_selection_enabled) {
    return std::abs(first.x - second.x) + std::abs(first.y - second.y) +
           std::abs(first.size - second.size) +
           (tilt_selection_enabled ? std::abs(first.tilt_x - second.tilt_x) +
                                         std::abs(first.tilt_y - second.tilt_y)
                                   : 0.0);
}

bool meets_minimum_progress(double progress, double minimum_progress) {
    return progress + k_progress_comparison_epsilon >= minimum_progress;
}

}  // namespace

selector::selector(data::sample_selector_config config)
    : m_config(std::move(config)) {}

data::sample_selection_result selector::try_add(
    data::target_detection detection) {
    if (is_duplicate(detection.selection)) {
        return data::sample_selection_result{
            .rejection_reason = data::sample_rejection_reason::duplicate,
            .progress = compute_progress(),
        };
    }

    m_features.push_back(detection.selection);
    m_observations.push_back(std::move(detection.calibration));
    return data::sample_selection_result{
        .accepted = true,
        .progress = compute_progress(),
    };
}

size_t selector::accepted_count() const noexcept {
    return m_observations.size();
}

const std::vector<data::observation>& selector::observations() const noexcept {
    return m_observations;
}

const std::vector<data::sample_features>& selector::features() const noexcept {
    return m_features;
}

data::coverage_progress selector::progress() const {
    return compute_progress();
}

void selector::set_tilt_selection_enabled(const bool enabled) noexcept {
    m_tilt_selection_enabled = enabled;
}

bool selector::replace_features(std::vector<data::sample_features> features) {
    if (features.size() != m_observations.size()) {
        return false;
    }
    m_features = std::move(features);
    return true;
}

data::coverage_progress selector::compute_progress() const {
    if (m_features.empty()) {
        return {};
    }

    data::sample_features minimum = m_features.front();
    data::sample_features maximum = m_features.front();
    for (const auto& feature : m_features) {
        minimum.x = std::min(minimum.x, feature.x);
        minimum.y = std::min(minimum.y, feature.y);
        minimum.size = std::min(minimum.size, feature.size);
        minimum.tilt_x = std::min(minimum.tilt_x, feature.tilt_x);
        minimum.tilt_y = std::min(minimum.tilt_y, feature.tilt_y);

        maximum.x = std::max(maximum.x, feature.x);
        maximum.y = std::max(maximum.y, feature.y);
        maximum.size = std::max(maximum.size, feature.size);
        maximum.tilt_x = std::max(maximum.tilt_x, feature.tilt_x);
        maximum.tilt_y = std::max(maximum.tilt_y, feature.tilt_y);
    }

    minimum.size = 0.0;

    const auto calculate_progress = [](double minimum, double maximum,
                                       double required_range) {
        if (required_range <= 0.0) {
            return 1.0;
        }
        return std::clamp((maximum - minimum) / required_range, 0.0, 1.0);
    };

    data::coverage_progress result{
        .x = calculate_progress(minimum.x, maximum.x,
                                m_config.required_ranges[0]),
        .y = calculate_progress(minimum.y, maximum.y,
                                m_config.required_ranges[1]),
        .size = calculate_progress(minimum.size, maximum.size,
                                   m_config.required_ranges[2]),
        .tilt_x = m_tilt_selection_enabled
                      ? calculate_progress(minimum.tilt_x, maximum.tilt_x,
                                           m_config.required_ranges[3])
                      : 1.0,
        .tilt_y = m_tilt_selection_enabled
                      ? calculate_progress(minimum.tilt_y, maximum.tilt_y,
                                           m_config.required_ranges[4])
                      : 1.0,
    };
    result.data_sufficient =
        m_features.size() >= m_config.minimum_samples_for_completion &&
        meets_minimum_progress(result.x, m_config.minimum_progress[0]) &&
        meets_minimum_progress(result.y, m_config.minimum_progress[1]) &&
        meets_minimum_progress(result.size, m_config.minimum_progress[2]) &&
        (!m_tilt_selection_enabled ||
         (meets_minimum_progress(result.tilt_x, m_config.minimum_progress[3]) &&
          meets_minimum_progress(result.tilt_y, m_config.minimum_progress[4])));
    return result;
}

bool selector::is_duplicate(const data::sample_features& candidate) const {
    return std::any_of(m_features.begin(), m_features.end(),
                       [this, &candidate](const auto& existing) {
                           return distance(candidate, existing,
                                           m_tilt_selection_enabled) <=
                                  m_config.duplicate_distance_threshold;
                       });
}

}  // namespace clover2_camera_utils::calibration::sample
