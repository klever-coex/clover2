#pragma once

#include <clover2/aruco/optimizer/base_optimizer.hpp>
#include <clover2/common/util/time_buffer.hpp>
#include <rclcpp/timer.hpp>

namespace clover2::aruco::optimizer {

class graph_linearizer : public base_optimizer {
public:
    using time_buffer_type =
        clover2::common::util::time_buffer<marker, std::chrono::nanoseconds>;

    static constexpr const char* name = "graph_linearizer";

    explicit graph_linearizer(const clover2::aruco::optimizer::context& ctx);
    virtual ~graph_linearizer();

    void optimize() override;

    void push_measurement(std::string& source_frame,
                          std::chrono::nanoseconds timestamp,
                          std::vector<marker>& measurement) override;
    void clear_measurements() override;

private:
    void initialize_time_buffer();
    void initialize_timer();
    void on_timer();

    void declare_parameters();
    void undeclare_parameters();

    std::string m_source_frame;
    std::chrono::nanoseconds m_last_timestamp;
    std::chrono::nanoseconds m_period;
    rclcpp::TimerBase::SharedPtr m_timer;
    std::shared_ptr<time_buffer_type> m_measurements;
};

}  // namespace clover2::aruco::optimizer
