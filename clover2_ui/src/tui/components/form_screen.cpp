#include <clover2_ui/tui/components/form_screen.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <cpptui/cpptui.hpp>

#include <memory>
#include <string>
#include <vector>

namespace clover2_ui::tui::components {

form_screen::form_screen(std::shared_ptr<api::config_field> object,
                         navigate_fn on_navigate,
                         std::shared_ptr<core::navigator> nav)
    : core::screen(object->name(), std::move(nav))
    , m_on_navigate(std::move(on_navigate))
    , m_object(std::move(object)) {
    m_title_label = std::make_shared<cpptui::Label>(
        m_object->name(), cpptui::Theme::current().primary);
    m_title_label->fixed_height = 1;
    add(m_title_label);

    add(std::make_shared<cpptui::HorizontalSpacer>());

    for (size_t i = 0; i < m_object->children().size(); i++) {
        const auto& field = m_object->children()[i];

        auto btn = std::make_shared<cpptui::Button>(
            cpptui::StyledText(field->name()), [this, i]() {
                if (m_on_navigate) {
                    m_on_navigate(m_object->children()[i]);
                }
            });

        btn->fixed_height = 1;
        btn->alignment = cpptui::Alignment::Left;
        add(btn);

        m_buttons.push_back(std::move(btn));
    }
}

void form_screen::on_focus() {
    if (!m_buttons.empty()) {
        m_buttons[m_current]->set_focus(true);
    }
}

void form_screen::on_enter() { on_focus(); }

bool form_screen::on_event(const cpptui::Event& event) {
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

std::vector<std::pair<std::string, std::string>> form_screen::shortcuts() const {
    return {{"up/down", "Navigate"}};
}

}  // namespace clover2_ui::tui::components
