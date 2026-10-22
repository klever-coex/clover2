#pragma once

#include <clover2_camera_utils/calibration/algorithm/base.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration::algorithm {

class factory {
public:
    using value_type = std::unique_ptr<base>;
    using builder_type = std::function<value_type()>;

    static factory& instance();

    template <typename T>
    void add(const std::string& method) {
        static_assert(std::is_base_of_v<base, T>,
                      "T must be derived from base");
        builders_[method] = []() { return std::make_unique<T>(); };
    }

    bool contains(const std::string& method) const;
    value_type create(const std::string& method) const;
    std::vector<std::string> list_methods() const;

private:
    factory();

    std::unordered_map<std::string, builder_type> builders_;
};

}  // namespace clover2_camera_utils::calibration::algorithm