/**
 * @file factory.hpp
 * @brief Provides construction of calibration targets from board profiles.
 */

#pragma once

#include <clover2_camera_utils/calibration/target/base.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration::target {

/** @brief Creates prepared calibration targets by type identifier. */
class factory {
public:
    using value_type = std::shared_ptr<const base>;
    using builder_type = std::function<value_type(const nlohmann::json&)>;

    /** @brief Returns the shared factory instance. */
    static factory& instance();

    /**
     * @brief Registers a target using its class type identifier.
     * @tparam T Target type derived from base with `name` and `from_json`.
     */
    template <typename T>
    void add() {
        static_assert(std::is_base_of_v<base, T>,
                      "T must be derived from base");
        builders_[std::string(T::name)] = &T::from_json;
    }

    /**
     * @brief Checks whether a target type is registered.
     * @param type Calibration target type identifier.
     * @return `true` if the type is supported, otherwise `false`.
     */
    bool contains(std::string_view type) const;

    /**
     * @brief Creates a target from a self-describing JSON profile file.
     * @param path Absolute path to a JSON target profile.
     * @return Immutable target prepared for image processing.
     * @throw std::invalid_argument If the path, profile envelope, or target
     * profile is invalid.
     * @throw std::runtime_error If the file cannot be read or parsed.
     */
    value_type create(const std::filesystem::path& path) const;

    /**
     * @brief Returns identifiers of registered target types.
     * @return List of supported calibration target types.
     */
    std::vector<std::string> list_types() const;

private:
    factory();

    std::unordered_map<std::string, builder_type> builders_;
};

}  // namespace clover2_camera_utils::calibration::target
