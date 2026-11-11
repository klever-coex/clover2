/**
 * @file charuco.hpp
 * @brief Provides a ChArUco calibration target.
 */

#pragma once

#include <clover2_camera_utils/calibration/target/base.hpp>

#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>

#include <cstdint>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>

namespace clover2_camera_utils::calibration::target {

/** @brief Detects and visualizes an interpolated ChArUco target. */
class charuco final : public base {
public:
    static constexpr std::string_view name{"charuco"};

    /**
     * @brief Creates a prepared ChArUco target from its JSON profile.
     * @param profile Parsed ChArUco target profile.
     * @return Immutable calibration target.
     */
    static std::shared_ptr<const base> from_json(
        const nlohmann::json& profile);

    /**
     * @brief Creates a prepared ChArUco target.
     * @param squares_rows Number of squares by rows.
     * @param squares_columns Number of squares by columns.
     * @param square_size_m Square side length in meters.
     * @param marker_size_m ArUco marker side length in meters.
     * @param dictionary Predefined ArUco dictionary name.
     * @throw std::invalid_argument If the dictionary is unsupported.
     */
    charuco(uint32_t squares_rows, uint32_t squares_columns,
            double square_size_m, double marker_size_m, std::string dictionary);

    data::target_detection_result detect(
        const cv::Mat& image) const override;

    void draw_debug(
        cv::Mat& image, const data::target_debug& debug) const override;

private:
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    cv::Ptr<cv::aruco::CharucoBoard> m_board;
};

}  // namespace clover2_camera_utils::calibration::target
