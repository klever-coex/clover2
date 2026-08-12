#pragma once

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <diagnostic_msgs/msg/key_value.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace clover2_ui::api::diagnostics {

struct tree_node {
    std::string name;
    std::string path;
    std::string message;
    std::string hardware_id;
    std::uint8_t level = diagnostic_msgs::msg::DiagnosticStatus::STALE;
    std::vector<diagnostic_msgs::msg::KeyValue> values;
    std::vector<tree_node> children;
    bool has_status = false;
};

struct snapshot {
    std::shared_ptr<const tree_node> root;
    std::array<std::size_t, 4> counts = {0, 0, 0, 0};
    std::uint8_t worst_level = diagnostic_msgs::msg::DiagnosticStatus::STALE;
    bool received = false;
    double age_sec = 0.0;
};

class model {
public:
    void update(const diagnostic_msgs::msg::DiagnosticArray& msg);
    snapshot get_snapshot() const;

private:
    static std::vector<std::string> split_path(const std::string& name);
    static std::uint8_t worst(std::uint8_t lhs, std::uint8_t rhs);
    static void update_worst_levels(tree_node& node);

    std::shared_ptr<const tree_node> m_root;
    std::array<std::size_t, 4> m_counts = {0, 0, 0, 0};
    std::uint8_t m_worst_level = diagnostic_msgs::msg::DiagnosticStatus::STALE;
    bool m_received = false;
};

std::string level_name(std::uint8_t level);

}  // namespace clover2_ui::api::diagnostics
