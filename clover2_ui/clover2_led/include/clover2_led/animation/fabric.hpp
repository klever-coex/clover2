#pragma once

// clover2
#include <clover2_led/animation/base_animation.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_led::animation {

class fabric {
public:
    using value_type = std::unique_ptr<base_animation>;
    using builder_type =
        std::function<value_type(const base_animation::Request&, int led_count,
                                 rclcpp::Clock::SharedPtr)>;

    static fabric& instance();

    template <typename T>
    void add() {
        static_assert(std::is_base_of_v<base_animation, T>,
                      "T must be derived from base_animation");
        m_builders[T::name] = [](const base_animation::Request& req,
                                 int led_count,
                                 rclcpp::Clock::SharedPtr clock) {
            return std::make_unique<T>(req, led_count, std::move(clock));
        };
    }

    value_type create(const std::string& name,
                      const base_animation::Request& req, int led_count,
                      rclcpp::Clock::SharedPtr clock) const;

    std::vector<std::string> list_animations() const;

private:
    fabric();
    std::unordered_map<std::string, builder_type> m_builders;
};

}  // namespace clover2_led::animation
