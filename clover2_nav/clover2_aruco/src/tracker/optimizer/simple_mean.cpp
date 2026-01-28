// clover2
#include <clover2/aruco/optimizer/simple_mean.hpp>
#include <clover2/common/util/time_buffer.hpp>

// ROS2
#include <rclcpp/logging.hpp>

namespace clover2::aruco::optimizer {

simple_mean::simple_mean(const clover2::aruco::optimizer::context& ctx)
    : base_optimizer(ctx)
    , m_source_frame("") {}

void simple_mean::optimize() {
    if (m_measurements->empty()) {
        return;
    }

    auto avg_cov = Eigen::Matrix<double, 6, 6>::Zero();
    auto avg_translation = Eigen::Vector3d::Zero();
    auto cumulative_q = Eigen::Vector4d::Zero();

    time_buffer_type buffer_copy(*m_measurements);

    for (const auto& [timestamp, marker] : buffer_copy.buffer()) {
        avg_translation += marker.transform.translation();
        Eigen::Quaterniond q(marker.transform.rotation());
        cumulative_q += q.coeffs();
        avg_cov += marker.cov;
    }

    double count = static_cast<double>(buffer_copy.size());
    avg_cov /= count;
    avg_translation /= count;
    cumulative_q /= count;

    auto avg_quat = Eigen::Quaterniond::Identity();
    if (cumulative_q.norm() > 0.0) {
        avg_quat.coeffs() = cumulative_q.normalized();
    }

    marker result;
    result.transform = Eigen::Affine3d::Identity();
    result.transform.translate(avg_translation);
    result.transform.rotate(avg_quat);
    result.cov = avg_cov;

    notify_data_ready(result, buffer_copy.back().first);
}

void simple_mean::push_measurement(std::string& source_frame,
                                   std::chrono::nanoseconds timestamp,
                                   std::vector<marker>& measurement) {
    if (m_source_frame == "") {
        m_source_frame = source_frame;
    } else if (m_source_frame != source_frame) {
        RCLCPP_ERROR(m_logger,
                     "Simple optimizer supports only one source frame");
        return;
    }

    m_source_frame = source_frame;

    for (const auto& it : measurement) {
        m_measurements->add(timestamp, it);
    }
}

void simple_mean::clear_measurements() { m_measurements->clear(); }

}  // namespace clover2::aruco::optimizer
