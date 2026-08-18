#include <clover2_ui/api/diagnostics/diagnostic_model.hpp>

#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>

namespace clover2_ui::api::diagnostics {

std::string level_name(std::uint8_t level) {
    switch (level) {
        case diagnostic_msgs::msg::DiagnosticStatus::OK:
            return "OK";
        case diagnostic_msgs::msg::DiagnosticStatus::WARN:
            return "WARN";
        case diagnostic_msgs::msg::DiagnosticStatus::ERROR:
            return "ERROR";
        case diagnostic_msgs::msg::DiagnosticStatus::STALE:
            return "STALE";
        default:
            return "UNKNOWN";
    }
}

std::vector<std::string> model::split_path(const std::string& name) {
    std::vector<std::string> result;
    std::stringstream ss(name);
    std::string part;

    while (std::getline(ss, part, '/')) {
        if (!part.empty()) result.push_back(part);
    }

    if (result.empty() && !name.empty()) result.push_back(name);
    return result;
}

std::uint8_t model::worst(std::uint8_t lhs, std::uint8_t rhs) {
    auto rank = [](std::uint8_t level) {
        switch (level) {
            case diagnostic_msgs::msg::DiagnosticStatus::ERROR:
                return 4;
            case diagnostic_msgs::msg::DiagnosticStatus::STALE:
                return 3;
            case diagnostic_msgs::msg::DiagnosticStatus::WARN:
                return 2;
            case diagnostic_msgs::msg::DiagnosticStatus::OK:
                return 1;
            default:
                return 5;
        }
    };

    return rank(lhs) >= rank(rhs) ? lhs : rhs;
}

void model::update_worst_levels(tree_node& node) {
    for (auto& child : node.children) {
        update_worst_levels(child);
        node.level = worst(node.level, child.level);
    }
}

void model::update(
    const diagnostic_msgs::msg::DiagnosticArray& msg) {
    tree_node root;
    root.name = "Diagnostics";
    root.path = "";
    root.level = diagnostic_msgs::msg::DiagnosticStatus::OK;
    m_counts = {0, 0, 0, 0};
    m_worst_level = diagnostic_msgs::msg::DiagnosticStatus::OK;
    m_received = true;

    for (const auto& status : msg.status) {
        if (status.level < m_counts.size()) m_counts[status.level]++;
        m_worst_level = worst(m_worst_level, status.level);

        auto* current = &root;
        std::string path;
        for (const auto& part : split_path(status.name)) {
            path += "/" + part;
            auto it = std::find_if(
                current->children.begin(), current->children.end(),
                [&](const tree_node& n) { return n.name == part; });

            if (it == current->children.end()) {
                tree_node child;
                child.name = part;
                child.path = path;
                child.level = diagnostic_msgs::msg::DiagnosticStatus::OK;
                current->children.push_back(std::move(child));
                it = std::prev(current->children.end());
            }

            current = &(*it);
        }

        current->has_status = true;
        current->level = status.level;
        current->message = status.message;
        current->hardware_id = status.hardware_id;
        current->values = status.values;
    }

    update_worst_levels(root);
    m_root = std::make_shared<tree_node>(std::move(root));
}

snapshot model::get_snapshot() const {
    snapshot out;
    out.root = m_root;
    out.counts = m_counts;
    out.worst_level = m_worst_level;
    out.received = m_received;
    return out;
}

}  // namespace clover2_ui::api::diagnostics
