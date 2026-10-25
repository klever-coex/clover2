#pragma once

// clover2
#include <clover2/aruco/solver/base_solver.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2::aruco::solver {

class factory {
public:
    using value_type = base_solver::UniquePtr;
    using builder_type =
        std::function<value_type(base_solver::required_interfaces)>;

    static factory& instance();

    template <typename T>
    void add() {
        static_assert(std::is_base_of_v<base_solver, T>,
                      "T must be derived from base_solver");
        m_builders[T::name] =
            [](base_solver::required_interfaces node_interfaces) {
                return std::make_unique<T>(std::move(node_interfaces));
            };
    }

    value_type create(const std::string& name,
                      base_solver::required_interfaces node_interfaces) const;

    std::vector<std::string> list_solvers() const;

private:
    factory();
    std::unordered_map<std::string, builder_type> m_builders;
};

}  // namespace clover2::aruco::solver
