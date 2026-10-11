#pragma once

// clover2
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>
#include <clover2_common/util/parameter.hpp>
#include <clover2_map/data/map.hpp>
#include <clover2_pose_msgs/msg/marker_frame.hpp>

// ROS2
#include <rclcpp/node_interfaces/node_interfaces.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/rclcpp.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <memory>
#include <optional>

namespace clover2::aruco::solver {

class base_solver {
public:
    using UniquePtr = std::unique_ptr<base_solver>;
    using required_interfaces = rclcpp::node_interfaces::NodeInterfaces<
        rclcpp::node_interfaces::NodeParametersInterface>;

    explicit base_solver(required_interfaces node_interfaces);
    virtual ~base_solver();

    virtual std::optional<Eigen::Isometry3d> solve(
        const clover2_pose_msgs::msg::MarkerFrame& frame,
        const clover2_map::map& map) = 0;

protected:
    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        clover2_common::util::ParameterFunctorT cb,
        const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        rcl_interfaces::msg::ParameterDescriptor descriptor;

        descriptor.name = name;
        descriptor.description = description;
        descriptor.additional_constraints = additional_constraints;
        descriptor.read_only = read_only;
        descriptor.dynamic_typing = true;

        clover2_common::util::declare_and_watch_parameter<ParameterT>(
            *m_parameters_watcher, name, default_value, cb, descriptor,
            ignore_override);
    }

private:
    clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
        m_parameters_watcher;
};

}  // namespace clover2::aruco::solver
