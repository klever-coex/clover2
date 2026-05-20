#pragma once

#include <boost/sml.hpp>

namespace clover2_fcu_bridge::fsm {

struct fsm_table {
    auto operator()() const noexcept {

        return boost::sml::make_transition_table();
    };
};

}
