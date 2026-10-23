/**
 * @file factory.hpp
 * @brief Provides an algorithm factory for camera calibration.
 */

#pragma once

#include <clover2_camera_utils/calibration/algorithm/base.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration::algorithm {

/** @brief Creates camera calibration algorithms by method identifier. */
class factory {
public:
    using value_type = std::unique_ptr<base>;
    using builder_type = std::function<value_type()>;

    /** @brief Returns the shared factory instance. */
    static factory& instance();

    /**
     * @brief Registers an algorithm for the specified method.
     * @tparam T Algorithm type derived from base.
     * @param method Calibration method identifier.
     */
    template <typename T>
    void add(const std::string& method) {
        static_assert(std::is_base_of_v<base, T>,
                      "T must be derived from base");
        builders_[method] = []() { return std::make_unique<T>(); };
    }

    /**
     * @brief Checks whether a method is registered.
     * @param method Calibration method identifier.
     * @return `true` if the method is supported, otherwise `false`.
     */
    bool contains(const std::string& method) const;

    /**
     * @brief Creates an algorithm for the specified method.
     * @param method Calibration method identifier.
     * @return New algorithm instance.
     * @throw std::invalid_argument If the method is not registered.
     */
    value_type create(const std::string& method) const;

    /**
     * @brief Returns identifiers of registered methods.
     * @return List of supported calibration methods.
     */
    std::vector<std::string> list_methods() const;

private:
    factory();

    std::unordered_map<std::string, builder_type> builders_;
};

}  // namespace clover2_camera_utils::calibration::algorithm
