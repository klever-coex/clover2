#include <clover2_camera_utils/calibration/sample_selector.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace clover2_camera_utils::calibration {
namespace {

double distance(const data::sample_features& first,
                const data::sample_features& second) {
    return std::abs(first.x - second.x) + std::abs(first.y - second.y) +
           std::abs(first.size - second.size) +
           std::abs(first.skew - second.skew);
}

}  // namespace

sample_selector::sample_selector(data::sample_selector_config config)
    : m_config(std::move(config)) {}

data::sample_selection_result sample_selector::try_add(
    data::observation observation, const data::board_config& board) {
    const auto candidate = sample_features_calculator{}.compute(
        observation.image_points, board, observation.image_size);
    if (!candidate.has_value()) {
        return data::sample_selection_result{
            .rejection_reason =
                data::sample_rejection_reason::invalid_observation,
            .progress = compute_progress(),
        };
    }

    if (is_duplicate(*candidate)) {
        return data::sample_selection_result{
            .rejection_reason = data::sample_rejection_reason::duplicate,
            .progress = compute_progress(),
        };
    }

    m_features.push_back(*candidate);
    m_observations.push_back(std::move(observation));
    return data::sample_selection_result{
        .accepted = true,
        .progress = compute_progress(),
    };
}

size_t sample_selector::accepted_count() const noexcept {
    return m_observations.size();
}

const std::vector<data::observation>& sample_selector::observations()
    const noexcept {
    return m_observations;
}

const std::vector<data::sample_features>& sample_selector::features()
    const noexcept {
    return m_features;
}

data::coverage_progress sample_selector::progress() const {
    return compute_progress();
}

data::coverage_progress sample_selector::compute_progress() const {
    if (m_features.empty()) {
        return {};
    }

    data::sample_features minimum = m_features.front();
    data::sample_features maximum = m_features.front();
    for (const auto& feature : m_features) {
        minimum.x = std::min(minimum.x, feature.x);
        minimum.y = std::min(minimum.y, feature.y);
        minimum.size = std::min(minimum.size, feature.size);
        minimum.skew = std::min(minimum.skew, feature.skew);

        maximum.x = std::max(maximum.x, feature.x);
        maximum.y = std::max(maximum.y, feature.y);
        maximum.size = std::max(maximum.size, feature.size);
        maximum.skew = std::max(maximum.skew, feature.skew);
    }

    minimum.size = 0.0;
    minimum.skew = 0.0;

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
        .skew = calculate_progress(minimum.skew, maximum.skew,
                                   m_config.required_ranges[3]),
    };
    result.data_sufficient =
        m_features.size() >= m_config.minimum_samples_for_completion ||
        (result.x == 1.0 && result.y == 1.0 && result.size == 1.0 &&
         result.skew == 1.0);
    return result;
}

bool sample_selector::is_duplicate(
    const data::sample_features& candidate) const {
    return std::any_of(m_features.begin(), m_features.end(),
                       [this, &candidate](const auto& existing) {
                           return distance(candidate, existing) <=
                                  m_config.duplicate_distance_threshold;
                       });
}

}  // namespace clover2_camera_utils::calibration
