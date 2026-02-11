// clover2
#include <clover2/aruco/optimizer/simple_mean.hpp>
#include <clover2/common/util/time_buffer.hpp>

// ROS2
#include <rclcpp/logging.hpp>

namespace clover2::aruco::optimizer {

simple_mean::simple_mean(const clover2::aruco::optimizer::context& ctx)
    : base_optimizer(ctx)
    , m_source_frame("") {
    declare_parameters();

    initialize_time_buffer();
}

simple_mean::~simple_mean() { undeclare_parameters(); }

void simple_mean::optimize() {
    if (m_measurements->empty()) {
        return;
    }

    RCLCPP_DEBUG(m_logger, "Optimizing using %zu measurements",
                m_measurements->size());
    Eigen::Vector3d avg_translation = Eigen::Vector3d::Zero();
    Eigen::Vector4d cumulative_q = Eigen::Vector4d::Zero();

    time_buffer_type buffer_copy(*m_measurements);

    for (const auto& [timestamp, marker] : buffer_copy.buffer()) {
        RCLCPP_DEBUG(m_logger, "  Marker ID: %d", marker.id);
        avg_translation += marker.transform.translation();
        Eigen::Quaterniond q(marker.transform.rotation());
        cumulative_q += q.coeffs();
    }

    RCLCPP_DEBUG(m_logger, "Averaged over %zu measurements", buffer_copy.size());
    double count = static_cast<double>(buffer_copy.size());
    avg_translation /= count;
    cumulative_q /= count;

    RCLCPP_DEBUG(m_logger, "Computed average translation: [%f, %f, %f]",
                avg_translation.x(), avg_translation.y(), avg_translation.z());
    Eigen::Quaterniond avg_quat = Eigen::Quaterniond::Identity();
    if (cumulative_q.norm() > 0.0) {
        avg_quat.coeffs() = cumulative_q.normalized();
    }

    RCLCPP_DEBUG(m_logger,
                "Computed average rotation quaternion: [%f, %f, %f, %f]",
                avg_quat.x(), avg_quat.y(), avg_quat.z(), avg_quat.w());
    marker result;
    result.transform = Eigen::Affine3d::Identity();
    result.transform.translate(avg_translation);
    result.transform.rotate(avg_quat);
    result.cov = buffer_copy.back().second.cov;

    notify_data_ready(result, buffer_copy.back().first);
}

void simple_mean::push_measurements(const std::string& source_frame,
                                    const std::chrono::nanoseconds timestamp,
                                    const std::vector<marker>& measurements) {
    if (m_source_frame == "") {
        m_source_frame = source_frame;
    } else if (m_source_frame != source_frame) {
        RCLCPP_ERROR(m_logger,
                     "Simple optimizer supports only one source frame");
        return;
    }

    m_source_frame = source_frame;
    m_measurements->add(timestamp, measurements);
}

void simple_mean::clear_measurements() { m_measurements->clear(); }

void simple_mean::initialize_time_buffer() {
    auto history_parameter =
        m_ctx.node_parameters->get_parameter("simple_mean.history_sec");

    if (history_parameter.get_type() !=
        rclcpp::ParameterType::PARAMETER_DOUBLE) {
        throw std::runtime_error("history_sec should be double");
    }

    RCLCPP_DEBUG(m_logger, "Init time buffer with %.3f second history",
                 history_parameter.as_double());

    auto history_sec = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(history_parameter.as_double()));

    m_measurements = std::make_shared<time_buffer_type>(history_sec);
}

void simple_mean::declare_parameters() {
    m_ctx.node_parameters->declare_parameter("simple_mean.history_sec",
                                             rclcpp::ParameterValue(0.5));
}

void simple_mean::undeclare_parameters() {
    m_ctx.node_parameters->undeclare_parameter("simple_mean.history_sec");
}

}  // namespace clover2::aruco::optimizer
