#pragma once

// clover2
#include <clover2_fcu/context.hpp>

// STL
#include <string>

namespace clover2_fcu_mavros {

inline std::string topic_prefix(const clover2_fcu::context& ctx) {
    const auto backend = ctx.params("mavros");
    if (backend && backend["namespace"]) {
        const auto ns = backend["namespace"].as<std::string>();
        if (!ns.empty()) {
            return ns;
        }
    }

    return "/mavros";
}

}  // namespace clover2_fcu_mavros
