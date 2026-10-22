#pragma once

#include <clover2_camera_utils/calibration/detector/base.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration::detector {

class factory {
public:
    using value_type = std::shared_ptr<const base>;
    using builder_type = std::function<value_type()>;

    static factory& instance();

    template <typename T>
    void add(const std::string& board_type) {
        static_assert(std::is_base_of_v<base, T>,
                      "T must be derived from base");
        builders_[board_type] = []() { return std::make_shared<T>(); };
    }

    bool contains(const std::string& board_type) const;
    value_type create(const std::string& board_type) const;
    std::vector<std::string> list_board_types() const;

private:
    factory();

    std::unordered_map<std::string, builder_type> builders_;
};

}  // namespace clover2_camera_utils::calibration::detector
