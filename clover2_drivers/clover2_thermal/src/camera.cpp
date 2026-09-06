#include <clover2_thermal/camera.hpp>

namespace clover2_thermal {

camera::camera(const rclcpp::NodeOptions& options)
    : clover2_common::node("camera", options) {}

camera::~camera() = default;

}  // namespace clover2_thermal

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_thermal::camera)
