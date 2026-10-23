/**
 * @file factory.hpp
 * @brief Provides a detector factory for calibration boards.
 */

#pragma once

#include <clover2_camera_utils/calibration/detector/base.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration::detector {

/** @brief Creates calibration board detectors by board type identifier. */
class factory {
public:
    using value_type = std::shared_ptr<const base>;
    using builder_type = std::function<value_type()>;

    /** @brief Returns the shared factory instance. */
    static factory& instance();

    /**
     * @brief Registers a detector for the specified board type.
     * @tparam T Detector type derived from base.
     * @param board_type Board type identifier.
     */
    template <typename T>
    void add(const std::string& board_type) {
        static_assert(std::is_base_of_v<base, T>,
                      "T must be derived from base");
        builders_[board_type] = []() { return std::make_shared<T>(); };
    }

    /**
     * @brief Checks whether a board type is registered.
     * @param board_type Board type identifier.
     * @return `true` if the board type is supported, otherwise `false`.
     */
    bool contains(const std::string& board_type) const;

    /**
     * @brief Creates a detector for the specified board type.
     * @param board_type Board type identifier.
     * @return New detector instance.
     * @throw std::invalid_argument If the board type is not registered.
     */
    value_type create(const std::string& board_type) const;

    /**
     * @brief Returns identifiers of registered board types.
     * @return List of supported board types.
     */
    std::vector<std::string> list_board_types() const;

private:
    factory();

    std::unordered_map<std::string, builder_type> builders_;
};

}  // namespace clover2_camera_utils::calibration::detector
