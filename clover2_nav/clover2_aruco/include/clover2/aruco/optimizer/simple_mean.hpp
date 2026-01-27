#pragma once

#include <clover2/aruco/optimizer/base_optimizer.hpp>

namespace clover2::aruco::optimizer {

class simple_mean : public base_optimizer {
public:
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
    std::chrono::nanoseconds m_timestamp;

    std::vector<marker> m_measurements;
};

}  // namespace clover2::aruco::optimizer
