#include <clover2_ui/tui/components/diagnostics_screen.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <iomanip>
#include <sstream>

namespace clover2_ui::tui::components {

namespace diagnostics_api = clover2_ui::api::diagnostics;

diagnostics_screen::diagnostics_screen(
    std::shared_ptr<diagnostics_api::monitor> monitor,
    std::shared_ptr<core::navigator> nav)
    : core::screen("Diagnostics", std::move(nav))
    , m_monitor(std::move(monitor)) {
    set_can_go_back(false);

    m_summary_label =
        std::make_shared<cpptui::Label>("Waiting for diagnostics...");
    m_summary_label->fixed_height = 1;
    add(m_summary_label);

    m_filters = std::make_shared<cpptui::CheckboxList>();
    m_filters->set_options({"ERROR", "WARN", "STALE", "OK"});
    m_filters->checked_states = {true, true, true, false};
    m_filters->fixed_height = 4;
    m_filters->on_change = [this](int, bool) { rebuild_view(); };
    add(m_filters);

    auto body = std::make_shared<cpptui::Horizontal>();
    add(body);

    auto list_border =
        std::make_shared<cpptui::Border>(cpptui::BorderStyle::Single);
    list_border->set_title("Diagnostic list", cpptui::Alignment::Left);
    body->add(list_border);

    m_list = std::make_shared<cpptui::Vertical>();
    list_border->add(m_list);

    auto details_border =
        std::make_shared<cpptui::Border>(cpptui::BorderStyle::Single);
    details_border->set_title("Details", cpptui::Alignment::Left);
    body->add(details_border);

    m_details = std::make_shared<cpptui::Paragraph>();
    details_border->add(m_details);
}

void diagnostics_screen::on_enter() {
    refresh_from_monitor();
    if (!m_buttons.empty()) m_buttons[m_selected]->set_focus(true);
}

bool diagnostics_screen::on_event(const cpptui::Event& event) {
    if (event.is_key_event()) {
        if (event.is_nav_up()) {
            move_selection(-1);
            return true;
        }
        if (event.is_nav_down()) {
            move_selection(1);
            return true;
        }
        if (event.is_activate() || event.is_nav_right() ||
            event.is_nav_left()) {
            toggle_selected();
            return true;
        }
    }

    return core::screen::on_event(event);
}

std::vector<std::pair<std::string, std::string>> diagnostics_screen::shortcuts()
    const {
    return {{"up/down", "Navigate"},
            {"space", "Collapse/expand"},
            {"mouse", "Toggle filters"}};
}

void diagnostics_screen::refresh_from_monitor() {
    if (!m_monitor) return;
    m_snapshot = m_monitor->get_snapshot();
    rebuild_view();
}

cpptui::Color diagnostics_screen::level_color(std::uint8_t level) {
    switch (level) {
        case diagnostic_msgs::msg::DiagnosticStatus::OK:
            return cpptui::Theme::current().success;
        case diagnostic_msgs::msg::DiagnosticStatus::WARN:
            return cpptui::Theme::current().warning;
        case diagnostic_msgs::msg::DiagnosticStatus::ERROR:
            return cpptui::Theme::current().error;
        case diagnostic_msgs::msg::DiagnosticStatus::STALE:
        default:
            return cpptui::Theme::current().foreground;
    }
}

bool diagnostics_screen::is_group(
    const diagnostics_api::tree_node& node) {
    return !node.children.empty();
}

void diagnostics_screen::rebuild_view() {
    std::ostringstream summary;
    if (!m_snapshot.received) {
        summary << "STALE | waiting for " << m_monitor->topic();
    } else {
        summary << diagnostics_api::level_name(m_snapshot.worst_level)
                << " | OK " << m_snapshot.counts[0] << " WARN "
                << m_snapshot.counts[1] << " ERROR " << m_snapshot.counts[2]
                << " STALE " << m_snapshot.counts[3] << " | last update "
                << std::fixed << std::setprecision(1) << m_snapshot.age_sec
                << "s ago | " << m_monitor->topic();
    }
    m_summary_label->set_text(cpptui::StyledText().colored(
        summary.str(), level_color(m_snapshot.worst_level)));

    m_visible_items.clear();
    if (m_snapshot.received && m_snapshot.root) {
        for (const auto& child : m_snapshot.root->children) {
            rebuild_visible_items(child, 0);
        }
    }

    if (m_selected >= static_cast<int>(m_visible_items.size())) {
        m_selected = static_cast<int>(m_visible_items.size()) - 1;
    }
    if (m_selected < 0) m_selected = 0;

    update_list_buttons();
    update_details();
}

void diagnostics_screen::rebuild_visible_items(
    const diagnostics_api::tree_node& node, int depth) {
    const bool group = is_group(node);
    if (filter_accepts(node.level) || group) {
        m_visible_items.push_back({&node, depth, group});
    }

    if (group && m_collapsed.count(node.path) == 0) {
        for (const auto& child : node.children) {
            rebuild_visible_items(child, depth + 1);
        }
    }
}

void diagnostics_screen::update_list_buttons() {
    m_list->clear_children();
    m_buttons.clear();

    if (m_visible_items.empty()) {
        auto empty = std::make_shared<cpptui::Label>(
            m_snapshot.received ? "No diagnostics match current filters"
                                : "No messages received yet");
        empty->fixed_height = 1;
        m_list->add(empty);
        return;
    }

    for (std::size_t i = 0; i < m_visible_items.size(); ++i) {
        const auto& item = m_visible_items[i];
        if (!item.node) continue;

        const bool collapsed = m_collapsed.count(item.node->path) > 0;
        std::string prefix(item.depth * 2, ' ');
        prefix += item.has_children ? (collapsed ? "> " : "v ") : "  ";

        auto text =
            cpptui::StyledText()
                .add(prefix)
                .add(item.node->name + " ")
                .colored(
                    "[" + diagnostics_api::level_name(item.node->level) + "]",
                    level_color(item.node->level));

        auto button = std::make_shared<cpptui::Button>(text, [this, i]() {
            m_selected = static_cast<int>(i);
            update_list_buttons();
            update_details();
        });
        button->alignment = cpptui::Alignment::Left;
        button->fixed_height = 1;
        m_list->add(button);
        m_buttons.push_back(button);
    }

    if (!m_buttons.empty()) m_buttons[m_selected]->set_focus(true);
}

void diagnostics_screen::update_details() {
    const auto* item = selected_item();
    if (!item || !item->node) {
        m_details->set_text("Select a diagnostic item to see details.");
        return;
    }

    const auto& node = *item->node;
    std::ostringstream out;
    out << "Name: " << node.path << std::endl;
    out << "Level: " << diagnostics_api::level_name(node.level) << std::endl;
    if (!node.message.empty()) out << "Message: " << node.message << std::endl;
    if (!node.hardware_id.empty()) {
        out << "Hardware ID: " << node.hardware_id << std::endl;
    }

    if (!node.values.empty()) {
        out << std::endl << "Values:" << std::endl;
        for (const auto& kv : node.values) {
            out << "  " << kv.key << ": " << kv.value << std::endl;
        }
    } else if (item->has_children) {
        out << std::endl
            << "Group contains " << node.children.size()
            << " child diagnostics.";
    }

    m_details->set_text(out.str());
}

bool diagnostics_screen::filter_accepts(std::uint8_t level) const {
    if (!m_filters || m_filters->checked_states.size() < 4) return true;
    switch (level) {
        case diagnostic_msgs::msg::DiagnosticStatus::ERROR:
            return m_filters->checked_states[0];
        case diagnostic_msgs::msg::DiagnosticStatus::WARN:
            return m_filters->checked_states[1];
        case diagnostic_msgs::msg::DiagnosticStatus::STALE:
            return m_filters->checked_states[2];
        case diagnostic_msgs::msg::DiagnosticStatus::OK:
            return m_filters->checked_states[3];
        default:
            return true;
    }
}

const diagnostics_screen::visible_item* diagnostics_screen::selected_item()
    const {
    if (m_selected < 0 ||
        m_selected >= static_cast<int>(m_visible_items.size())) {
        return nullptr;
    }
    return &m_visible_items[m_selected];
}

void diagnostics_screen::move_selection(int delta) {
    if (m_visible_items.empty()) return;
    m_selected += delta;
    if (m_selected < 0)
        m_selected = static_cast<int>(m_visible_items.size()) - 1;
    if (m_selected >= static_cast<int>(m_visible_items.size())) m_selected = 0;
    update_list_buttons();
    update_details();
}

void diagnostics_screen::toggle_selected() {
    const auto* item = selected_item();
    if (!item || !item->has_children) return;

    if (!item->node) return;

    if (m_collapsed.count(item->node->path)) {
        m_collapsed.erase(item->node->path);
    } else {
        m_collapsed.insert(item->node->path);
    }
    rebuild_view();
}

}  // namespace clover2_ui::tui::components
