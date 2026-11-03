/**
 * @file chessboard.hpp
 * @brief Provides a chessboard calibration target.
 */

#pragma once

#include <clover2_camera_utils/calibration/target/base.hpp>

#include <opencv2/core/types.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

namespace clover2_camera_utils::calibration::target {

/** @brief Detects and visualizes a conventional chessboard target. */
class chessboard final : public base {
public:
    static constexpr std::string_view name{"chessboard"};

    /**
     * @brief Loads a chessboard profile and creates its prepared target.
     * @param path Absolute path to a JSON chessboard profile.
     * @return Immutable calibration target.
     */
    static std::shared_ptr<const base> from_file(
        const std::filesystem::path& path);

    /**
     * @brief Creates a prepared chessboard target.
     * @param rows Number of inner corners by rows.
     * @param columns Number of inner corners by columns.
     * @param square_size_m Square side length in meters.
     */
    chessboard(uint32_t rows, uint32_t columns, double square_size_m);

    std::optional<data::observation> detect(
        const cv::Mat& image) const override;

    void draw_debug(
        cv::Mat& image,
        const std::optional<data::observation>& observation) const override;

private:
    cv::Size m_pattern_size;
    std::vector<cv::Point3f> m_object_points;
};

}  // namespace clover2_camera_utils::calibration::target
