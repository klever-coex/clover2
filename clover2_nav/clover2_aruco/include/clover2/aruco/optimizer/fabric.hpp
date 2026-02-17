#pragma once

// clover2
#include <clover2/aruco/optimizer/base_optimizer.hpp>

// STL
#include <functional>
#include <string>
#include <unordered_map>

namespace clover2::aruco::optimizer {

class fabric {
public:
    using value_type = std::shared_ptr<base_optimizer>;
    using builder_type = std::function<value_type(const context&)>;

    static fabric& instance();

    template <typename T>
    void add() {
        static_assert(std::is_base_of<base_optimizer, T>::value,
                      "T must be derived from base_optimizer");
        m_optimizers[T::name] = [](const context& ctx) {
            return std::make_shared<T>(ctx);
        };
    }

    value_type create(const std::string& name, const context& ctx) const;
    std::vector<std::string> list_optimizers() const;

private:
    fabric();

    std::unordered_map<std::string, builder_type> m_optimizers;
};

}  // namespace clover2::aruco::optimizer
