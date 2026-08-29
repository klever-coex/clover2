#include <clover2_notification/output.hpp>
#include <gtest/gtest.h>

#include <functional>
#include <vector>

namespace {

using clover2_notification::data::event;

class fake_output final : public clover2_notification::output {
public:
    void finish_current() {
        m_done();
    }

    const std::vector<event>& processed() const {
        return m_processed;
    }

protected:
    void on_initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr&) override {}

    void process_event(const event& value, done_callback done) override {
        m_processed.push_back(value);
        m_done = std::move(done);
    }

private:
    std::vector<event> m_processed;
    done_callback m_done;
};

TEST(output, processes_events_sequentially_in_descending_priority_order) {
    fake_output output;

    output.push2queue({1, "test", "warn", ""});
    output.push2queue({3, "test", "stale", ""});
    output.push2queue({2, "test", "error", ""});

    ASSERT_EQ(output.processed().size(), 1U);
    EXPECT_EQ(output.processed()[0].name, "warn");

    output.finish_current();
    ASSERT_EQ(output.processed().size(), 2U);
    EXPECT_EQ(output.processed()[1].name, "stale");

    output.finish_current();
    ASSERT_EQ(output.processed().size(), 3U);
    EXPECT_EQ(output.processed()[2].name, "error");
}

TEST(output, clear_discards_pending_events) {
    fake_output output;

    output.push2queue({1, "test", "current", ""});
    output.push2queue({3, "test", "pending", ""});
    output.clear();

    output.push2queue({2, "test", "new", ""});
    ASSERT_EQ(output.processed().size(), 2U);
    EXPECT_EQ(output.processed()[1].name, "new");
}

}  // namespace
