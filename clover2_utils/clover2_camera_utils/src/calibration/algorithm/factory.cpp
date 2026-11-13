#include <clover2_camera_utils/calibration/algorithm/factory.hpp>

#include <clover2_camera_utils/calibration/algorithm/fisheye.hpp>
#include <clover2_camera_utils/calibration/algorithm/pinhole.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace clover2_camera_utils::calibration::algorithm {

factory::factory() {
    add<pinhole>();
    add<fisheye>();
}

factory& factory::instance() {
    static factory instance;
    return instance;
}

bool factory::contains(const std::string& method) const {
    return builders_.contains(method);
}

factory::value_type factory::create(const std::string& method) const {
    const auto iterator = builders_.find(method);
    if (iterator == builders_.end()) {
        std::ostringstream message;
        message << "unknown calibration method '" << method << "'; available:";
        for (const auto& name : list_methods()) {
            message << " " << name;
        }
        throw std::invalid_argument(message.str());
    }
    return iterator->second();
}

std::vector<std::string> factory::list_methods() const {
    std::vector<std::string> methods;
    methods.reserve(builders_.size());
    for (const auto& [method, builder] : builders_) {
        static_cast<void>(builder);
        methods.push_back(method);
    }
    std::sort(methods.begin(), methods.end());
    return methods;
}

}  // namespace clover2_camera_utils::calibration::algorithm