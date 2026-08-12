#pragma once

#include <clover2_ui/api/diagnostics/diagnostic_model.hpp>
#include <clover2_ui/api/diagnostics/diagnostic_monitor.hpp>
#include <clover2_ui/tui/core/screen.hpp>
#include <cpptui/cpptui.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace clover2_ui::tui::components {

class diagnostics_screen : public core::screen {
public:
    explicit diagnostics_screen(
        std::shared_ptr<api::diagnostics::monitor> monitor,
        std::shared_ptr<core::navigator> nav);

    void on_enter() override;
    bool on_event(const cpptui::Event& event) override;
    std::vector<std::pair<std::string, std::string>> shortcuts() const override;

    void refresh_from_monitor();

private:
    struct visible_item {
        const api::diagnostics::tree_node* node = nullptr;
        int depth = 0;
        bool has_children = false;
    };

    static cpptui::Color level_color(std::uint8_t level);
    static bool is_group(const api::diagnostics::tree_node& node);

    void rebuild_view();
    void rebuild_visible_items(
        const api::diagnostics::tree_node& node, int depth);
    void update_list_buttons();
    void update_details();
    bool filter_accepts(std::uint8_t level) const;
    const visible_item* selected_item() const;
    void move_selection(int delta);
    void toggle_selected();

    std::shared_ptr<api::diagnostics::monitor> m_monitor;
    api::diagnostics::snapshot m_snapshot;

    std::shared_ptr<cpptui::Label> m_summary_label;
    std::shared_ptr<cpptui::CheckboxList> m_filters;
    std::shared_ptr<cpptui::Vertical> m_list;
    std::shared_ptr<cpptui::Paragraph> m_details;

    std::vector<std::shared_ptr<cpptui::Button>> m_buttons;
    std::vector<visible_item> m_visible_items;
    std::set<std::string> m_collapsed;
    int m_selected = 0;
};

}  // namespace clover2_ui::tui::components
