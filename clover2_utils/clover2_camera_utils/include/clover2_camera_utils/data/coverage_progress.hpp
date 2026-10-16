#pragma once

namespace clover2_camera_utils::data {

struct coverage_progress {
    double x{0.0};
    double y{0.0};
    double size{0.0};
    double skew{0.0};
    bool data_sufficient{false};
};

}  // namespace clover2_camera_utils::data
