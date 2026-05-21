#pragma once

#include <clover2_fcu_bridge/backend/base_backend.hpp>
#include <clover2_fcu_bridge/backend/context.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_fcu_bridge::backend {

class fabric {
public:
    using value_type = std::shared_ptr<base_backend>;
    using builder_type = std::function<value_type(const context&)>;

    static fabric& instance();

    template <typename T>
    void add() {
        static_assert(std::is_base_of_v<base_backend, T>,
                      "T must be derived from base_backend");
        m_builders[T::name] = [](const context& ctx) {
            return std::make_shared<T>(ctx);
        };
    }

    value_type create(const std::string& name, const context& ctx) const;
    std::vector<std::string> list_backends() const;

private:
    fabric();

    std::unordered_map<std::string, builder_type> m_builders;
};

}  // namespace clover2_fcu_bridge::backend
