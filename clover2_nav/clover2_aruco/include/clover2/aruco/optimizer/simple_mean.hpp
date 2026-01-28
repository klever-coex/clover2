#pragma once

// clover2
#include <clover2/aruco/optimizer/base_optimizer.hpp>

namespace clover2::common::util {
template <typename ValueT, typename TimeT>
class time_buffer;
}  // namespace clover2::common::util

namespace clover2::aruco::optimizer {

class simple_mean : public base_optimizer {
public:
    using time_buffer_type =
        clover2::common::util::time_buffer<marker, std::chrono::nanoseconds>;

    static constexpr const char* name = "simple_mean";

    explicit simple_mean(const clover2::aruco::optimizer::context& ctx);
    virtual ~simple_mean() = default;

    void optimize() override;

    void push_measurement(std::string& source_frame,
                          std::chrono::nanoseconds timestamp,
                          std::vector<marker>& measurement) override;
    void clear_measurements() override;

private:
    std::string m_source_frame;
    std::shared_ptr<time_buffer_type> m_measurements;
};

}  // namespace clover2::aruco::optimizer
