#pragma once

// clover2
#include <clover2/offboard/bridge/base_bridge.hpp>
#include <clover2/offboard/bridge/creation_context.hpp>

// STL
#include <string>
#include <vector>
#include <unordered_map>

namespace clover2::offboard::bridge {

class fabric {
public:
    using value_type = std::shared_ptr<base_bridge>;
    using builder_type = std::function<value_type(const creation_context&)>;

    static fabric& instance();

    template <typename T>
    void add() {
        static_assert(std::is_base_of<base_bridge, T>::value,
                      "T must be derived from base_bridge");
        m_bridges[T::name] = [](const creation_context& ctx) {
            return std::make_shared<T>(ctx);
        };
    }

    value_type create(const std::string& name, const creation_context& ctx) const;
    std::vector<std::string> list_bridges() const;

private:
    fabric();

    std::unordered_map<std::string, builder_type> m_bridges;
};

}  // namespace clover2::offboard::bridge
