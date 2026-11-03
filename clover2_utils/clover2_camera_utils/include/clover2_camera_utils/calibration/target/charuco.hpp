/**
 * @file charuco.hpp
 * @brief Provides a ChArUco calibration target.
 */

#pragma once

#include <clover2_camera_utils/calibration/target/base.hpp>

#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace clover2_camera_utils::calibration::target {

/** @brief Detects and visualizes an interpolated ChArUco target. */
class charuco final : public base {
public:
    static constexpr std::string_view name{"charuco"};

    /**
     * @brief Loads a ChArUco profile and creates its prepared target.
     * @param path Absolute path to a JSON ChArUco profile.
     * @return Immutable calibration target.
     */
    static std::shared_ptr<const base> from_file(
        const std::filesystem::path& path);

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

    std::optional<data::observation> detect(
        const cv::Mat& image) const override;

    void draw_debug(
        cv::Mat& image,
        const std::optional<data::observation>& observation) const override;

private:
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    cv::Ptr<cv::aruco::CharucoBoard> m_board;
};

}  // namespace clover2_camera_utils::calibration::target
