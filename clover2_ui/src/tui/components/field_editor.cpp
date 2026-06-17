#include <clover2_ui/tui/components/field_editor.hpp>
#include <clover2_ui/tui/core/navigator.hpp>

#include <cpptui/cpptui.hpp>

#include <memory>
#include <string>

namespace clover2_ui::tui::components {

field_editor::field_editor(std::shared_ptr<clover2_ui::api::config_field> field,
                         std::shared_ptr<core::navigator> nav)
    : core::screen(field->name(), std::move(nav))
    , m_field(std::move(field)) {
    m_title_label = std::make_shared<cpptui::Label>(
        m_field->name() + " (" + m_field->type_str() + ")",
        cpptui::Theme::current().primary);
    m_title_label->fixed_height = 1;
    add(m_title_label);

    m_desc_label = std::make_shared<cpptui::Label>(m_field->description(),
                                                    cpptui::Color());
    m_desc_label->fixed_height = 1;
    add(m_desc_label);

    add(std::make_shared<cpptui::HorizontalSpacer>());

    const auto& type = m_field->type_str();

    if (type == "bool") {
        bool initial = m_field->as<bool>();
        m_toggle = std::make_shared<cpptui::ToggleSwitch>(
            cpptui::StyledText(m_field->name()), initial);
        m_toggle->fixed_height = 1;
        add(m_toggle);
    } else if (type == "int") {
        int initial = m_field->as<int>();
        m_number_input = std::make_shared<cpptui::NumberInput>(initial);
        m_number_input->fixed_height = 1;
        add(m_number_input);
    } else if (type == "float") {
        m_input = std::make_shared<cpptui::Input>();
        m_input->fixed_height = 1;
        if (m_field->value().IsDefined() && !m_field->value().IsNull()) {
            m_input->set_value(std::to_string(m_field->as<double>()));
        } else if (m_field->has_default()) {
            m_input->set_value(
                std::to_string(m_field->default_value().as<double>()));
        }
        add(m_input);
    } else {
        m_input = std::make_shared<cpptui::Input>();
        m_input->fixed_height = 1;
        if (m_field->value().IsDefined() && !m_field->value().IsNull()) {
            m_input->set_value(m_field->as<std::string>());
        } else if (m_field->has_default()) {
            m_input->set_value(
                m_field->default_value().as<std::string>());
        }
        add(m_input);
    }

    add(std::make_shared<cpptui::HorizontalSpacer>());

    m_save_btn =
        std::make_shared<cpptui::Button>(cpptui::StyledText("[ Save ]"),
                                         [this]() { save(); });
    m_save_btn->fixed_height = 1;
    add(m_save_btn);

    m_cancel_btn = std::make_shared<cpptui::Button>(
        cpptui::StyledText("[ Cancel ]"), [this]() {
            if (auto nav = get_navigator()) nav->pop();
        });
    m_cancel_btn->fixed_height = 1;
    add(m_cancel_btn);
}

void field_editor::save() {
    const auto& type = m_field->type_str();

    if (type == "bool") {
        if (m_toggle) m_field->set(m_toggle->is_on);
    } else if (type == "int") {
        if (m_number_input) m_field->set(m_number_input->get_value());
    } else if (type == "float") {
        if (m_input) {
            try {
                double val = std::stod(m_input->get_value());
                m_field->set(val);
            } catch (...) {
            }
        }
    } else if (m_field->is_enum()) {
        if (m_input) {
            m_field->set_enum(m_input->get_value());
        }
    } else {
        if (m_input) m_field->set(m_input->get_value());
    }

    if (auto nav = get_navigator()) nav->pop();
}

void field_editor::on_focus() {
    if (m_input) m_input->set_focus(true);
    if (m_number_input) m_number_input->set_focus(true);
    if (m_toggle) m_toggle->set_focus(true);
}

void field_editor::on_enter() {
    on_focus();
}

bool field_editor::on_event(const cpptui::Event& event) {
    return core::screen::on_event(event);
}

}  // namespace clover2_ui::tui::components
