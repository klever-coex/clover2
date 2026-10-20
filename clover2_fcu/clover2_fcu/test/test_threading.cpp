// clover2
#include <clover2_common/node.hpp>
#include <clover2_common/util/shared_callback.hpp>
#include <clover2_fcu/backend/backend_base.hpp>
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// third party
#include <gtest/gtest.h>

// STL
#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

namespace {

/// @brief rclcpp init/shutdown for the whole binary.
class ros_env : public ::testing::Environment {
public:
    void SetUp() override { rclcpp::init(0, nullptr); }
    void TearDown() override { rclcpp::shutdown(); }
};

[[maybe_unused]] const auto* g_env =
    ::testing::AddGlobalTestEnvironment(new ros_env());

using clover2_common::util::shared_callback;

// ---------------------------------------------------------------------------
// shared_callback: concurrent registration and invocation.

TEST(shared_callback, concurrent_add_invoke_release) {
    shared_callback<void()> point;
    auto owner = std::make_shared<int>(0);
    std::atomic<int> fired{0};
    std::atomic<bool> stop{false};

    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&] {
            while (!stop) {
                auto token = point.add(owner, [&] { ++fired; });
                point.invoke();
                token.release();  // release mid-flight of other threads
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    stop = true;
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_GT(fired.load(), 0);
}

TEST(shared_callback, reentrant_add_and_self_unregister) {
    shared_callback<void()> point;
    auto owner = std::make_shared<int>(0);

    int late_fires = 0;
    shared_callback<void()>::token first_token;
    shared_callback<void()>::token late_token;

    first_token = point.add(owner, [&] {
        // Re-entrant registration while being invoked (token kept!)...
        late_token = point.add(owner, [&late_fires] { ++late_fires; });
        // ...and self-unregistration while being invoked.
        first_token.release();
    });

    point.invoke();  // fires the first callback only
    point.invoke();  // first is gone, the late one fires
    point.invoke();

    EXPECT_EQ(late_fires, 2);
}

// ---------------------------------------------------------------------------
// telemetry_interface: concurrent updates vs readers.

struct counter_snapshot {
    uint64_t seq{0};
    uint64_t mirror{0};  // == seq in every consistent snapshot
};

class counter_interface
    : public clover2_fcu::telemetry_interface<counter_snapshot> {
public:
    explicit counter_interface(
        clover2_fcu::capability::base_interface::shared_ptr base)
        : telemetry_interface(std::move(base)) {}

    void inject(uint64_t seq) { update_data(counter_snapshot{seq, seq}); }
};

TEST(telemetry_interface, concurrent_read_write) {
    auto node = std::make_shared<clover2_common::node>("telemetry_mt_test");
    auto base = std::make_shared<clover2_fcu::capability::base_interface_impl>(
        *node, "/fcu_bridge", nullptr);
    // on_update() requires shared_from_this(): the provider must be
    // owned by a shared_ptr, as it always is in the client cache.
    auto iface = std::make_shared<counter_interface>(base);

    std::atomic<bool> stop{false};
    std::atomic<uint64_t> fires{0};
    auto token = iface->on_update([&](const counter_snapshot&) { ++fires; });

    std::thread writer([&] {
        uint64_t seq = 0;
        while (!stop) {
            iface->inject(++seq);
        }
    });

    std::thread reader([&] {
        uint64_t observed = 0;
        while (!stop) {
            if (iface->has_data()) {
                // By-value snapshot: a torn read would break the mirror.
                const auto snap = iface->last();
                ASSERT_EQ(snap.seq, snap.mirror);
                ASSERT_GE(snap.seq, observed);
                observed = snap.seq;
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    stop = true;
    writer.join();
    reader.join();

    EXPECT_GT(fires.load(), 0u);
}

// ---------------------------------------------------------------------------
// backend_base: lazy cache builds each provider exactly once.

class test_provider
    : public clover2_fcu::capability::system_state_interface {
public:
    explicit test_provider(
        clover2_fcu::capability::base_interface::shared_ptr base)
        : system_state_interface(std::move(base)) {}
};

class test_backend : public clover2_fcu::backend::backend_base {
public:
    test_backend() {
        using clover2_fcu::capability::system_state_interface;

        add<system_state_interface>([this] {
            ++m_builds;
            // Widen the race window for concurrent first get() calls.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // Builders resolve dependencies re-entrantly via get().
            return std::make_shared<test_provider>(
                get<clover2_fcu::capability::base_interface>());
        });
    }

    std::string protocol() const override { return "test_mt"; }

    void do_initialize(
        std::shared_ptr<clover2_fcu::capability::base_interface>) override {}

    int m_builds{0};
};

TEST(backend_base, concurrent_get_builds_once) {
    auto node = std::make_shared<clover2_common::node>("backend_mt_test");
    auto base =
        std::make_shared<clover2_fcu::capability::base_interface_impl>(
            *node, "/fcu_bridge", nullptr);

    test_backend backend;
    backend.initialize(base);

    constexpr int k_threads = 8;
    std::vector<std::shared_ptr<void>> results(k_threads);
    std::vector<std::thread> threads;
    for (auto& r : results) {
        threads.emplace_back([&backend, &r] {
            r = backend.get(
                clover2_fcu::capability::system_state_interface::name);
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(backend.m_builds, 1);
    for (const auto& r : results) {
        EXPECT_EQ(r, results.front());
    }
}

}  // namespace
