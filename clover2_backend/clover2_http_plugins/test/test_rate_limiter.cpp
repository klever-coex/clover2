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
    rate_limiter limiter(100.0);

    EXPECT_TRUE(limiter.allow(64 * 1024));
    EXPECT_FALSE(limiter.allow(1));
}

TEST(RateLimiter, OversizeMessagePassesOnFullBucketOnly) {
    rate_limiter limiter(10.0);

    EXPECT_TRUE(limiter.allow(1 << 20));
    EXPECT_FALSE(limiter.allow(1 << 20));
    EXPECT_FALSE(limiter.allow(1));
}

}  // namespace
