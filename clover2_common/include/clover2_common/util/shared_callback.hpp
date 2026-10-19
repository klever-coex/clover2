/**
 * @file shared_callback.hpp
 * @brief Provides a shared callback point with RAII registration tokens.
 */

#pragma once

// STL
#include <functional>
#include <list>
#include <memory>
#include <utility>

namespace clover2_common::util {

/**
 * @brief Shared callback point with RAII registration tokens.
 *
 * Owns a list of callbacks; add() registers a callback and returns a
 * token whose destruction unregisters it, invoke() fires every
 * registered callback in registration order.
 *
 * add() takes the shared_ptr of the object owning this callback point
 * (typically the caller's shared_from_this()): tokens hold a weak guard
 * on it, so they safely outlive the callback point, and the owner cannot
 * be kept alive by its tokens. The callback point is neither copyable
 * nor movable (tokens are bound to one instance).
 *
 * Assumes a single-threaded executor (registration and invocation on the
 * same thread). A callback must not unregister itself from the callback
 * point it is invoked from (the iteration would break); it may
 * unregister other callbacks.
 *
 * @tparam Signature callback signature, default void().
 */
template <typename Signature = void()>
class shared_callback {
public:
    using callback_type = std::function<Signature>;

    /**
     * @brief Move-only RAII token of a registration.
     *
     * Destruction (or move assignment) unregisters the callback — but
     * only while the owning object is still alive: the token holds a
     * weak guard, so it may safely outlive the owner (unregistering
     * becomes a no-op). The release action receives the proven-alive
     * owner pointer, so it never touches a dangling object and cannot
     * extend the owner lifetime.
     *
     * Default-constructed tokens are empty (destruction is a no-op).
     */
    class token {
    public:
        token() = default;

        ~token() { release(); }

        token(const token&) = delete;
        token& operator=(const token&) = delete;

        token(token&& other) noexcept
            : m_guard(std::move(other.m_guard))
            , m_release(std::move(other.m_release)) {
            other.m_release = nullptr;
        }

        token& operator=(token&& other) noexcept {
            if (this != &other) {
                release();
                m_guard = std::move(other.m_guard);
                m_release = std::move(other.m_release);
                other.m_release = nullptr;
            }

            return *this;
        }

        /**
         * @brief Bind the token to an owner and its release action.
         * @param owner the object the registration lives in (weak guard).
         * @param release invoked with the owner pointer on destruction,
         *        only while the owner is alive.
         */
        template <typename Owner, typename Release>
        static token make(const std::shared_ptr<Owner>& owner,
                          Release release) {
            token instance;
            instance.m_guard = owner;
            instance.m_release = [release = std::move(release)](void* raw) {
                release(static_cast<Owner*>(raw));
            };

            return instance;
        }

        void release() {
            if (!m_release) {
                return;
            }

            if (const auto owner = m_guard.lock()) {
                m_release(owner.get());
            }

            m_release = nullptr;
        }

        /// @brief Whether the token holds a registration.
        explicit operator bool() const { return static_cast<bool>(m_release); }

    private:
        std::weak_ptr<void> m_guard;
        std::function<void(void*)> m_release;
    };

    shared_callback() = default;

    shared_callback(const shared_callback&) = delete;
    shared_callback& operator=(const shared_callback&) = delete;

    /**
     * @brief Register a callback.
     * @param owner shared_ptr of the object owning this callback point
     *        (the guard for the returned token; typically
     *        shared_from_this()).
     * @param callback the callback function.
     * @returns RAII token; keep it while registered.
     */
    template <typename Owner>
    token add(const std::shared_ptr<Owner>& owner, callback_type callback) {
        const auto iterator =
            m_callbacks.insert(m_callbacks.end(), std::move(callback));

        return token::make(
            owner, [this, iterator](Owner*) { m_callbacks.erase(iterator); });
    }

    /**
     * @brief Invoke every registered callback in registration order.
     * @param args arguments forwarded to each callback.
     */
    template <typename... Args>
    void invoke(Args&&... args) {
        for (const auto& callback : m_callbacks) {
            callback(args...);
        }
    }

private:
    std::list<callback_type> m_callbacks;
};

}  // namespace clover2_common::util
