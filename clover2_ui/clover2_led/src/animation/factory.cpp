#include <clover2_led/animation/blink.hpp>
#include <clover2_led/animation/factory.hpp>
#include <clover2_led/animation/rainbow.hpp>
#include <clover2_led/animation/solid_color.hpp>

#include <sstream>
#include <stdexcept>

namespace clover2_led::animation {

factory::factory() {
    add<solid_color>();
    add<blink>();
    add<rainbow>();
}

factory& factory::instance() {
    static factory inst;
    return inst;
}

factory::value_type factory::create(const std::string& name,
                                  const base_animation::Request& req,
                                  int led_count,
                                  rclcpp::Clock::SharedPtr clock) const {
    auto it = m_builders.find(name);
    if (it == m_builders.end()) {
        std::ostringstream oss;
        oss << "unknown animation '" << name << "'; available:";
        for (const auto& n : list_animations()) {
            oss << " " << n;
        }
        throw std::runtime_error(oss.str());
    }

    return it->second(req, led_count, std::move(clock));
}

std::vector<std::string> factory::list_animations() const {
    std::vector<std::string> names;
    names.reserve(m_builders.size());
    for (const auto& pair : m_builders) {
        names.push_back(pair.first);
    }
    return names;
}

}  // namespace clover2_led::animation
