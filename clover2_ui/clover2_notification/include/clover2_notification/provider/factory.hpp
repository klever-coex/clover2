#pragma once

#include <clover2_notification/provider/base.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_notification::provider {

class factory {
public:
    using value_type = std::shared_ptr<base>;
    using builder_type = std::function<value_type()>;

    static factory& instance();

    template <typename T>
    void add() {
        static_assert(std::is_base_of_v<base, T>,
                      "T must be derived from provider::base");
        m_builders[T::name] = []() {
            return std::make_shared<T>();
        };
    }

    value_type create(const std::string& name) const;
    std::vector<std::string> list_providers() const;

private:
    factory();

    std::unordered_map<std::string, builder_type> m_builders;
};

}  // namespace clover2_notification::provider
