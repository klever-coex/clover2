#include <clover2_ui/api/settings/field_type.hpp>
#include <clover2_ui/tui/components/field_editor.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <cpptui/cpptui.hpp>

#include <memory>
#include <stdexcept>
#include <string>

namespace clover2_ui::tui::components {

field_editor::field_editor(
    std::shared_ptr<clover2_ui::api::settings::config_field> field,
    std::shared_ptr<core::navigator> nav)
    : core::screen(field->name(), std::move(nav))
    , m_field(std::move(field)) {
    m_title_label = std::make_shared<cpptui::Label>(
        m_field->name() + " (" + std::string(m_field->type_str()) + ")",
        cpptui::Theme::current().primary);
    m_title_label->fixed_height = 1;
    add(m_title_label);

    m_desc_label = std::make_shared<cpptui::Label>(m_field->description(),
                                                   cpptui::Color());
    m_desc_label->fixed_height = 1;
    add(m_desc_label);

    add(std::make_shared<cpptui::HorizontalSpacer>());

    const auto& type = m_field->type();

    if (type == api::settings::field_type::BOOL) {
        bool initial = m_field->as<bool>();
        m_toggle = std::make_shared<cpptui::ToggleSwitch>(
            cpptui::StyledText(m_field->name()), initial);
        m_toggle->fixed_height = 1;

        add(m_toggle);
    } else {
        m_input = std::make_shared<cpptui::Input>();
        m_input->fixed_height = 1;

        switch (type) {
            case api::settings::field_type::STR:
                m_input->set_value(m_field->as<std::string>());
                break;
            case api::settings::field_type::INT:
                m_input->set_value(std::to_string(m_field->as<int>()));
                break;
            case api::settings::field_type::FLOAT:
                m_input->set_value(std::to_string(m_field->as<double>()));
                break;
            default:
                break;
        }

        add(m_input);
    }

    add(std::make_shared<cpptui::HorizontalSpacer>());

    m_save_btn = std::make_shared<cpptui::Button>(cpptui::StyledText("Save"),
                                                  [this]() { save(); });
    m_save_btn->fixed_height = 1;
    add(m_save_btn);

    m_cancel_btn = std::make_shared<cpptui::Button>(
        cpptui::StyledText("Cancel"), [this]() {
            if (auto nav = get_navigator()) nav->pop();
        });

    m_cancel_btn->fixed_height = 1;
    add(m_cancel_btn);
}

void field_editor::save() noexcept {
    try {
        try_save();
        get_navigator()->pop();
    } catch (const std::exception& e) {
        get_navigator()->show_notification(
            e.what(), cpptui::Notification::Type::Error, 1000);
    }
}

void field_editor::try_save() {
    if (m_toggle) {
        m_field->set(m_toggle->is_on);
        return;
    }

    m_field->set_from_string(m_input->get_value());
}

void field_editor::on_focus() {
    if (m_input) m_input->set_focus(true);
    if (m_toggle) m_toggle->set_focus(true);
}

void field_editor::on_enter() { on_focus(); }

bool field_editor::on_event(const cpptui::Event& event) {
    if (event.is_nav_down()) {
        m_save_btn->set_focus(true);
        return true;
    }

    if (event.is_nav_up()) {
        m_cancel_btn->set_focus(true);
        return true;
    }

    return core::screen::on_event(event);
}

}  // namespace clover2_ui::tui::components
