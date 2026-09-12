#pragma once

#include <clover2_ui/api/diagnostics/diagnostic_model.hpp>
#include <clover2_ui/tui/core/screen.hpp>
#include <cpptui/cpptui.hpp>

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace clover2_ui::tui::components {

class diagnostics_screen : public core::screen {
public:
    using snapshot_provider = std::function<api::diagnostics::snapshot()>;

    explicit diagnostics_screen(
        snapshot_provider snapshots,
        std::string topic,
        std::shared_ptr<core::navigator> nav);

    void on_enter() override;
    bool on_event(const cpptui::Event& event) override;
    std::vector<std::pair<std::string, std::string>> shortcuts() const override;

    void refresh();

private:
    struct visible_item {
        const api::diagnostics::tree_node* node = nullptr;
        int depth = 0;
        bool has_children = false;
    };

    static cpptui::Color level_color(std::uint8_t level);
    static bool is_group(const api::diagnostics::tree_node& node);

    void rebuild_view();
    void rebuild_visible_items(const api::diagnostics::tree_node& node,
                               int depth);
    void update_list_buttons();
    void ensure_selected_visible();
    void update_details();
    bool filter_accepts(std::uint8_t level) const;
    const visible_item* selected_item() const;
    void move_selection(int delta);
    void toggle_selected();

    snapshot_provider m_snapshots;
    std::string m_topic;
    api::diagnostics::snapshot m_snapshot;

    std::shared_ptr<cpptui::Label> m_summary_label;
    std::shared_ptr<cpptui::CheckboxList> m_filters;
    std::shared_ptr<cpptui::ScrollableVertical> m_list;
    std::shared_ptr<cpptui::Paragraph> m_details;

    std::vector<std::shared_ptr<cpptui::Button>> m_buttons;
    std::vector<visible_item> m_visible_items;
    std::set<std::string> m_collapsed;
    int m_selected = 0;
};

}  // namespace clover2_ui::tui::components
