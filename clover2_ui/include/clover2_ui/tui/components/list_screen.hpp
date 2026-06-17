#pragma once

#include <clover2_ui/tui/core/screen.hpp>
#include <cpptui/cpptui.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace clover2_ui::tui::components {

struct ListItem {
    std::string name;
    std::string description;
    std::string suffix;
};

class list_screen : public core::screen {
public:
    using item_callback_fn = std::function<void(size_t index)>;

    list_screen(std::string name, std::vector<ListItem> items, item_callback_fn cb,
                std::shared_ptr<core::navigator> nav = nullptr);

    bool on_event(const cpptui::Event& event) override;
    void on_focus() override;

private:
    size_t m_current{0};

    item_callback_fn m_callback;

    std::vector<ListItem> m_items;
    std::shared_ptr<cpptui::Label> m_title_label;
    std::vector<std::shared_ptr<cpptui::Button>> m_buttons;
};

}  // namespace clover2_ui::tui::components
