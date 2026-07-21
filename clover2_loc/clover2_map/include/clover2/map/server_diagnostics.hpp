#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace clover2::map {

enum class MapServerDiagnostic {
    map,
    interface,
};

class MapServerDiagnostics
    : public clover2_common::node_interfaces::TypedNodeDiagnostics<
          MapServerDiagnostic> {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(MapServerDiagnostics)

    using diagnostic = MapServerDiagnostic;
    using Base = clover2_common::node_interfaces::TypedNodeDiagnostics<
        diagnostic>;
    using callback = Base::callback;

    MapServerDiagnostics(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
        rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
            logging_interface,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface,
        rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
            timers_interface,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
            topics_interface);

    virtual ~MapServerDiagnostics();

private:
    RCLCPP_DISABLE_COPY(MapServerDiagnostics)

    static const Base::DiagnosticNamesT diagnostic_names;
};

}  // namespace clover2::map
