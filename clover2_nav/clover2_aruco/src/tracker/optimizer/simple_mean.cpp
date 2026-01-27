#include <clover2/aruco/optimizer/simple_mean.hpp>
#include <rclcpp/logging.hpp>

namespace clover2::aruco::optimizer {

simple_mean::simple_mean(const clover2::aruco::optimizer::context& ctx)
    : base_optimizer(ctx)
    , m_source_frame("")
    , m_timestamp(std::chrono::nanoseconds(0)) {}

void simple_mean::optimize() {
    marker result;
    result.transform = Eigen::Affine3d::Identity();
    result.cov.setZero();

    if (!m_measurements.empty()) {
        Eigen::Vector3d avg_translation = Eigen::Vector3d::Zero();
        Eigen::Vector4d cumulative_q = Eigen::Vector4d::Zero();
        Eigen::Matrix<double, 6, 6> avg_cov =
            Eigen::Matrix<double, 6, 6>::Zero();

        for (const auto& measurement : m_measurements) {
            avg_translation += measurement.transform.translation();
            Eigen::Quaterniond q(measurement.transform.rotation());
            cumulative_q += q.coeffs();
            avg_cov += measurement.cov;
        }

        avg_translation /= static_cast<double>(m_measurements.size());
        cumulative_q /= static_cast<double>(m_measurements.size());
        avg_cov /= static_cast<double>(m_measurements.size());

        Eigen::Quaterniond avg_quat = Eigen::Quaterniond::Identity();
        if (cumulative_q.norm() > 0.0) {
            avg_quat.coeffs() = cumulative_q.normalized();
        }

        result.transform = Eigen::Affine3d::Identity();
        result.transform.translate(avg_translation);
        result.transform.rotate(avg_quat);
        result.cov = avg_cov;
    }

    notify_data_ready(result, m_timestamp);
    clear_measurements();
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
    m_timestamp = timestamp;
    m_measurements = measurement;
}

void simple_mean::clear_measurements() { m_measurements.clear(); }

}  // namespace clover2::aruco::optimizer
