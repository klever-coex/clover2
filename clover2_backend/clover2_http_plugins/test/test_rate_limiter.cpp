#include <clover2_http_plugins/utils/rate_limiter.hpp>

#include <gtest/gtest.h>

namespace {

using clover2_http_plugins::utils::rate_limiter;

TEST(RateLimiter, NonPositiveLimitDisablesThrottling) {
    rate_limiter unlimited(0.0);
    rate_limiter negative(-1.0);

    EXPECT_TRUE(unlimited.allow(1024));
    EXPECT_TRUE(unlimited.allow(1 << 20));
    EXPECT_TRUE(negative.allow(1024));
}

TEST(RateLimiter, BudgetExhaustedWithoutRefill) {
    // 100 bytes/s: the burst allowance is 64 KiB.
    rate_limiter limiter(100.0);

    EXPECT_TRUE(limiter.allow(64 * 1024));
    // The whole burst is consumed; without elapsed time nothing passes.
    EXPECT_FALSE(limiter.allow(1));
}

TEST(RateLimiter, OversizeMessagePassesOnFullBucketOnly) {
    // 10 bytes/s: the burst allowance is 64 KiB; a 1 MiB message is bigger
    // than the burst cap and passes only when the bucket is full.
    rate_limiter limiter(10.0);

    EXPECT_TRUE(limiter.allow(1 << 20));   // full bucket -> passes, drains it
    EXPECT_FALSE(limiter.allow(1 << 20));  // empty bucket -> dropped
    EXPECT_FALSE(limiter.allow(1));        // even small messages wait for refill
}

}  // namespace
