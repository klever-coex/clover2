#include <clover2/aruco/detail/g2o_types.hpp>
#include <clover2/aruco/optimizer/graph_linearizer.hpp>
#include <rclcpp/create_timer.hpp>
#include <rclcpp/logging.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>

#include <functional>

namespace clover2::aruco::optimizer {

graph_linearizer::graph_linearizer(
    const clover2::aruco::optimizer::context& ctx)
    : base_optimizer(ctx)
    , m_source_frame("")
    , m_last_timestamp(std::chrono::nanoseconds(0))
    , m_period(std::chrono::nanoseconds(0)) {
    declare_parameters();

    initialize_time_buffer();
    initialize_timer();
}

graph_linearizer::~graph_linearizer() {
    undeclare_parameters();
    m_measurements.reset();
}

void graph_linearizer::optimize() {}

void graph_linearizer::push_measurement(std::string& source_frame,
                                        std::chrono::nanoseconds timestamp,
                                        std::vector<marker>& measurement) {
    if (m_source_frame.empty()) {
        m_source_frame = source_frame;
    } else if (m_source_frame != source_frame) {
        RCLCPP_ERROR(m_logger,
                     "Graph linearizer supports only one source frame");
        return;
    }

    m_last_timestamp = timestamp;

    for (const auto& meas : measurement) {
        m_measurements->add(timestamp, meas);
    }
}

void graph_linearizer::clear_measurements() {
    m_measurements->clear();  //
}

void graph_linearizer::on_timer() {
    if (!m_measurements || m_measurements->empty()) {
        return;
    }

    time_buffer_type buffer_snapshot(*m_measurements);
    if (buffer_snapshot.empty()) {
        return;
    }

    // TODO(motya): Variant A g2o pipeline: build a single-pose graph using
    // g2o_types::VertexPose and EdgePoseMeasurement, synchronized via
    // time_buffer and timestamp-weighted covariance.

    marker result;
    notify_data_ready(result, m_last_timestamp);
}

void graph_linearizer::initialize_timer() {
    auto update_hz_parameter =
        m_ctx.node_parameters->get_parameter("graph_linearizer.update_hz");

    if (update_hz_parameter.get_type() !=
        rclcpp::ParameterType::PARAMETER_DOUBLE) {
        throw std::runtime_error("update_hz should be double");
    }

    RCLCPP_DEBUG(m_logger, "Set update rate to %.1f Hz",
                 update_hz_parameter.as_double());

    m_period = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(1.0 / update_hz_parameter.as_double()));

    m_timer = rclcpp::create_wall_timer(
        m_period, std::bind(&graph_linearizer::on_timer, this), nullptr,
        m_ctx.node_base.get(), m_ctx.node_timers.get());
}

void graph_linearizer::initialize_time_buffer() {
    auto history_parameter =
        m_ctx.node_parameters->get_parameter("graph_linearizer.history_sec");

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

void graph_linearizer::declare_parameters() {
    m_ctx.node_parameters->declare_parameter("graph_linearizer.history_sec",
                                             rclcpp::ParameterValue(0.5));
    m_ctx.node_parameters->declare_parameter("graph_linearizer.update_hz",
                                             rclcpp::ParameterValue(30.0));
}

void graph_linearizer::undeclare_parameters() {
    m_ctx.node_parameters->undeclare_parameter("graph_linearizer.history_sec");
    m_ctx.node_parameters->undeclare_parameter("graph_linearizer.update_hz");
}

}  // namespace clover2::aruco::optimizer
