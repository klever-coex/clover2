// clover2
#include <clover2_fcu/common/command_handle.hpp>

// third party
#include <gtest/gtest.h>

// STL
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

namespace {

using clover2_fcu::command_handle;
using clover2_fcu::result;

/// @brief Minimal transport keeper: holds the handle alive until it
/// resolves it (like an rclcpp::Client holding a response lambda).
class keeper {
public:
    void hold(command_handle::shared_ptr h) { m_handle = std::move(h); }

    void resolve(result r) {
        if (m_handle) {
            m_handle->complete(r);
            m_handle.reset();
        }
    }

private:
    command_handle::shared_ptr m_handle;
};

TEST(command_handle, on_done_after_terminal_fires_immediately) {
    auto h = std::make_shared<command_handle>();
    h->complete(result::value::success);

    int fires = 0;
    h->on_done([&](result r) {
        ++fires;
        EXPECT_EQ(r, result::value::success);
    });

    EXPECT_EQ(fires, 1);
}

TEST(command_handle, fire_and_forget_callback_survives_dropped_refs) {
    // The only external reference dies right after subscribing; the
    // keeper still owns the command, so the callback must arrive.
    keeper transport;
    std::atomic<bool> done{false};

    {
        auto h = std::make_shared<command_handle>();
        transport.hold(h);
        h->on_done([&](result r) {
            EXPECT_EQ(r, result::value::success);
            done = true;
        });
    }  // our reference dies; the command must not

    transport.resolve(result::value::success);

    EXPECT_TRUE(done.load());
}

TEST(command_handle, complete_is_idempotent_and_late_response_ignored) {
    auto h = std::make_shared<command_handle>();

    int fires = 0;
    result last{result::value::interrupted};
    h->on_done([&](result r) {
        ++fires;
        last = r;
    });

    h->complete(result::value::success);
    h->complete(result::value::rejected);  // late transport response
    h->cancel();                           // after terminal: no-op

    EXPECT_EQ(fires, 1);
    EXPECT_EQ(last, result::value::success);
}

TEST(command_handle, cancel_runs_abort_and_resolves_interrupted) {
    auto h = std::make_shared<command_handle>();

    int aborts = 0;
    h->attach_abort([&aborts] { ++aborts; });

    h->cancel();

    EXPECT_EQ(aborts, 1);
    EXPECT_TRUE(h->done());
    EXPECT_EQ(h->outcome(), result::value::interrupted);
}

TEST(command_handle, wait_for_blocking_completion) {
    auto h = std::make_shared<command_handle>();

    std::thread completer([h] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        h->complete(result::value::success);
    });
    completer.detach();

    const result r = h->wait_for(std::chrono::seconds(2));
    EXPECT_EQ(r, result::value::success);
}

TEST(command_handle, wait_for_timeout) {
    auto h = std::make_shared<command_handle>();
    EXPECT_EQ(h->wait_for(std::chrono::milliseconds(10)),
              result::value::timeout);
    EXPECT_FALSE(h->done());
}

TEST(command_handle, many_subscribers_all_fire) {
    auto h = std::make_shared<command_handle>();

    std::atomic<int> fires{0};
    for (int i = 0; i < 5; ++i) {
        h->on_done([&](result) { ++fires; });
    }

    h->complete(result::value::success);
    EXPECT_EQ(fires.load(), 5);
}

}  // namespace
