/**
 * @file factory.hpp
 * @brief Provides construction of calibration targets from board profiles.
 */

#pragma once

#include <clover2_camera_utils/calibration/target/base.hpp>

#include <filesystem>
#include <memory>
#include <string_view>

namespace clover2_camera_utils::calibration::target {

/** @brief Creates prepared calibration targets by type identifier. */
class factory {
public:
    /**
     * @brief Creates a target of the specified type from its JSON profile.
     * @param type Canonical target type identifier.
     * @param path Absolute path to a JSON board profile.
     * @return Immutable target prepared for image processing.
     * @throw std::invalid_argument If the type, path, or profile is invalid.
     * @throw std::runtime_error If the file cannot be read or parsed.
     */
    static std::shared_ptr<const base> create(
        std::string_view type,
        const std::filesystem::path& path);
};

}  // namespace clover2_camera_utils::calibration::target
