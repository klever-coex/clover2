#include <clover2_ui/tui/components/list_screen.hpp>
#include <cpptui/cpptui.hpp>

#include <memory>
#include <string>

namespace clover2_ui::tui::components {

list_screen::list_screen(std::string name, std::vector<ListItem> items,
                         item_callback_fn cb,
                         std::shared_ptr<core::navigator> nav)
    : core::screen(std::move(name), std::move(nav))
    , m_callback(std::move(cb))
    , m_items(std::move(items)) {
    m_title_label = std::make_shared<cpptui::Label>(
        m_name, cpptui::Theme::current().primary);
    m_title_label->fixed_height = 1;
    add(m_title_label);

    add(std::make_shared<cpptui::HorizontalSpacer>());

    for (size_t i = 0; i < m_items.size(); i++) {
        std::string label = m_items[i].name;
        if (!m_items[i].suffix.empty()) {
            label += "  " + m_items[i].suffix;
        }

        auto btn = std::make_shared<cpptui::Button>(
            cpptui::StyledText(label), [i, this]() { m_callback(i); });
        btn->fixed_height = 1;
        btn->alignment = cpptui::Alignment::Left;

        add(btn);

        m_buttons.push_back(std::move(btn));
    }
}

void list_screen::on_focus() {
    if (!m_buttons.empty()) {
        m_buttons[m_current]->set_focus(true);
    }
}

bool list_screen::on_event(const cpptui::Event& event) {
    if (event.is_nav_down()) {
        if (m_buttons.empty()) return true;
        if (m_current >= m_buttons.size() - 1) {
            m_current = 0;
        } else {
            m_current++;
        }
        m_buttons[m_current]->set_focus(true);
        return true;
    }

    if (event.is_nav_up()) {
        if (m_buttons.empty()) return true;
        if (m_current == 0) {
            m_current = m_buttons.size() - 1;
        } else {
            m_current--;
        }
        m_buttons[m_current]->set_focus(true);
        return true;
    }

    return core::screen::on_event(event);
}

std::vector<std::pair<std::string, std::string>> list_screen::shortcuts() const {
    return {{"up/down", "Navigate"}, {"enter", "Select"}};
}

}  // namespace clover2_ui::tui::components
