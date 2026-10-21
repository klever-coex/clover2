#pragma once

#include <clover2_camera_utils/calibration/board_detector.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration {

class board_detector_factory {
public:
    using value_type = std::shared_ptr<const board_detector>;
    using builder_type = std::function<value_type()>;

    static board_detector_factory& instance();

    template <typename T>
    void add(const std::string& board_type) {
        static_assert(std::is_base_of_v<board_detector, T>,
                      "T must be derived from board_detector");
        builders_[board_type] = []() { return std::make_shared<T>(); };
    }

    bool contains(const std::string& board_type) const;
    value_type create(const std::string& board_type) const;
    std::vector<std::string> list_board_types() const;

private:
    board_detector_factory();

    std::unordered_map<std::string, builder_type> builders_;
};

}  // namespace clover2_camera_utils::calibration
