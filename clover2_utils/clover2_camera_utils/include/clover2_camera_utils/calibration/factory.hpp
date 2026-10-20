#pragma once

#include <clover2_camera_utils/calibration/calibration_algorithm.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration {

class factory {
public:
    using value_type = std::unique_ptr<calibration_algorithm>;
    using builder_type = std::function<value_type()>;

    static factory& instance();

    template <typename T>
    void add(const std::string& method) {
        static_assert(std::is_base_of_v<calibration_algorithm, T>,
                      "T must be derived from calibration_algorithm");
        builders_[method] = []() { return std::make_unique<T>(); };
    }

    bool contains(const std::string& method) const;
    value_type create(const std::string& method) const;
    std::vector<std::string> list_methods() const;

private:
    factory();

    std::unordered_map<std::string, builder_type> builders_;
};

}  // namespace clover2_camera_utils::calibration