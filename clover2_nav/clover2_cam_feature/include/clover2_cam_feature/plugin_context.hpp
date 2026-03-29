#pragma once

#include <clover2/map_server/map_client.hpp>
#include <clover2/common/node_context.hpp>

#include <memory>

namespace clover2_cam_feature {

struct plugin_context : public clover2::common::node_context {
    template <typename NodeT>
    explicit plugin_context(NodeT& node)
        : clover2::common::node_context(node) {}

    std::shared_ptr<clover2::map_server::map_client> map_client;
};

}  // namespace clover2_cam_feature
