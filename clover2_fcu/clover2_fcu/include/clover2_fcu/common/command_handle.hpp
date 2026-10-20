/**
 * @file command_handle.hpp
 * @brief Provides the async command completion handle.
 */

#pragma once

// clover2
#include <clover2_common/util/shared_callback.hpp>
#include <clover2_fcu/common/result.hpp>

// STL
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

namespace clover2_fcu {

/**
 * @brief Async command completion handle.
 *
 * One handle per command call. The object is co-owned by the caller and
 * by whatever completes it — the transport keeper: a service client
 * response lambda (rclcpp::Client holds it until the response), a
 * telemetry-driven state machine, a deadline timer, the fake registry.
 * It stays alive until a terminal state, so:
 *
 *   auto h = commands->land();
 *   h->on_done(cb);   // the callback arrives: the keeper holds h
 *   // dropping the pointer = "no longer interested", NOT a cancel;
 *   // cancel() is the explicit abort.
 *
 * complete() is idempotent: a late transport response after
 * cancel()/timeout is ignored, no "cancelled but answered" branch
 * exists anywhere.
 *
 * Threading model: complete()/cancel() run on an executor thread (the
 * fcu callback group); wait_for() blocks a foreign thread (script/test
 * context — the completion needs the executor elsewhere) and must not
 * be called from the fcu group itself. on_done() subscribes from any
 * thread at any time; a handle already in a terminal state invokes the
 * callback immediately, outside any lock.
 *
 * Callbacks fire outside the internal lock and may re-enter
 * done()/outcome()/on_done(). A callback must not capture the handle
 * itself (owning capture = cycle: the callbacks live in the handle);
 * attach_abort() lambdas must go through a weak reference to the
 * provider.
 */
class command_handle : public std::enable_shared_from_this<command_handle> {
public:
    using shared_ptr = std::shared_ptr<command_handle>;
    using done_callback = std::function<void(result)>;
    using callbacks = clover2_common::util::shared_callback<void(result)>;

    /// @brief Terminal state of the command; nullopt while pending.
    std::optional<clover2_fcu::result> outcome() const {
        const std::lock_guard<std::mutex> lock(m_mutex);
        return m_result;
    }

    /// @brief Whether the command reached a terminal state.
    bool done() const { return outcome().has_value(); }

    /**
     * @brief Subscribe a completion callback (many allowed, no tokens:
     *        the subscription lives as long as the command).
     *
     * A handle already in a terminal state invokes the callback
     * immediately (on the calling thread).
     */
    void on_done(done_callback callback) {
        std::optional<clover2_fcu::result> already;
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_result) {
                // The token is kept for the handle lifetime: there is no
                // unsubscribe — the subscription lives as long as the
                // command (shared_callback is token-based by design).
                m_tokens.emplace_back(
                    m_callbacks.add(shared_from_this(), std::move(callback)));
                return;
            }
            already = *m_result;
        }

        callback(*already);
    }

    /**
     * @brief Wait for the completion (script/test contexts only).
     *
     * Blocks the calling thread; the completion is delivered by an
     * executor thread, so the node must be spun elsewhere. Never call
     * this from a callback of the fcu group — use on_done() there.
     *
     * @returns the terminal result, or result::timeout when the wait
     *          expired (the command stays pending).
     */
    template <typename rep_t, typename period_t>
    result wait_for(std::chrono::duration<rep_t, period_t> timeout) {
        std::unique_lock<std::mutex> lock(m_mutex);
        const bool finished =
            m_cv.wait_for(lock, timeout, [this] { return m_result.has_value(); });

        return finished ? *m_result : result{result::value::timeout};
    }

    /**
     * @brief Abort the command: runs the provider abort hook (if any)
     *        and resolves the handle with result::interrupted.
     *
     * A no-op for an already-terminal handle; providers register the
     * abort via attach_abort() (e.g. a mode switch for takeoff/land
     * tasks, nothing for request-flavored commands).
     */
    void cancel() {
        std::function<void()> abort;
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            if (m_result) {
                return;
            }
            abort = m_abort;
        }

        // Outside the lock: the abort is transport work (may itself
        // complete this handle re-entrantly).
        if (abort) {
            abort();
        }

        complete(result::value::interrupted);
    }

    /**
     * @brief Resolve the command (providers only).
     *
     * Idempotent and thread-safe; fires the callbacks outside the lock.
     */
    void complete(result r) {
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            if (m_result) {
                return;
            }
            m_result = r;
        }

        m_callbacks.invoke(r);
        m_cv.notify_all();
    }

    /**
     * @brief Register the abort action (providers only, before the
     *        handle is handed out).
     *
     * The lambda must not own the handle or the provider (weak
     * references only): it is stored for the whole command lifetime.
     */
    void attach_abort(std::function<void()> abort) {
        const std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_result) {
            m_abort = std::move(abort);
        }
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::optional<clover2_fcu::result> m_result;
    std::function<void()> m_abort;

    callbacks m_callbacks;
    std::list<callbacks::token> m_tokens;
};

}  // namespace clover2_fcu
