#pragma once

// clover2
#include <clover2_fcu/backend/backend_base.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <memory>
#include <string>

namespace clover2_fcu_mavros {

class factory : public clover2_fcu::backend::backend_base {
public:
    factory();

    std::string protocol() const override;

    void do_initialize(
        std::shared_ptr<capability::base_interface> base) override;
};

}  // namespace clover2_fcu_mavros
