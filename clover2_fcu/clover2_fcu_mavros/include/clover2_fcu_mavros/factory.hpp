#pragma once

// clover2
#include <clover2_fcu/backend/backend_base.hpp>

// STL
#include <string>

namespace clover2_fcu_mavros {

class factory : public clover2_fcu::backend::backend_base {
public:
    factory();

    std::string protocol() const override;

    void do_initialize() override;
};

}  // namespace clover2_fcu_mavros
