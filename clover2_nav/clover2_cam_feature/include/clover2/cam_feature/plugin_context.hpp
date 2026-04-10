#pragma once

#include <clover2/common/node_context.hpp>
#include <clover2/map/client.hpp>

#include <memory>

namespace clover2::cam_feature {

class plugin_context : public clover2::common::node_context {
public:
    template <typename NodeT>
    explicit plugin_context(NodeT& node)
        : clover2::common::node_context(node) {}

    rclcpp::Node* node;

    std::shared_ptr<clover2::map::client> map_client;
};

}  // namespace clover2::cam_feature
