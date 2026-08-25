/**
 * @file factory.hpp
 * @brief Provides notification provider factory.
 */

#pragma once

// clover2
#include <clover2_notification/provider/base.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_notification::provider {

/**
 * @class factory
 * @brief Factory for creating notification providers by name.
 */
class factory {
public:
    /** @brief Provider shared pointer type returned by the factory. */
    using value_type = std::shared_ptr<base>;

    /** @brief Callable type used to construct provider instances. */
    using builder_type = std::function<value_type()>;

    /**
     * @brief Get the singleton factory instance.
     *
     * @return Reference to the provider factory singleton.
     */
    static factory& instance();

    /**
     * @brief Register a provider type.
     *
     * @tparam T Provider type. Must inherit from provider::base and expose a
     * static name member.
     */
    template <typename T>
    void add() {
        static_assert(std::is_base_of_v<base, T>,
                      "T must be derived from provider::base");
        m_builders[T::name] = []() {
            return std::make_shared<T>();
        };
    }

    /**
     * @brief Create a provider by name.
     *
     * @param name Registered provider name.
     * @return New provider instance, or nullptr if @p name is unknown.
     */
    value_type create(const std::string& name) const;

    /**
     * @brief List registered provider names.
     *
     * @return Vector of registered provider names.
     */
    std::vector<std::string> list_providers() const;

private:
    /** @brief Construct the factory and register built-in providers. */
    factory();

    std::unordered_map<std::string, builder_type> m_builders;
};

}  // namespace clover2_notification::provider
