#pragma once

#include <clover2_ui/api/settings/config_field.hpp>
#include <clover2_ui/tui/core/screen.hpp>
#include <cpptui/cpptui.hpp>

#include <memory>
#include <string>

namespace clover2_ui::tui::components {

class field_editor : public core::screen {
public:
    field_editor(std::shared_ptr<api::settings::config_field> field,
                 std::shared_ptr<core::navigator> nav);

    bool on_event(const cpptui::Event& event) override;
    void on_focus() override;
    void on_enter() override;

private:
    void save() noexcept;
    void try_save();

    std::shared_ptr<api::settings::config_field> m_field;

    std::shared_ptr<cpptui::Label> m_title_label;
    std::shared_ptr<cpptui::Label> m_desc_label;
    std::shared_ptr<cpptui::Button> m_save_btn;
    std::shared_ptr<cpptui::Button> m_cancel_btn;

    std::shared_ptr<cpptui::Input> m_input{nullptr};
    std::shared_ptr<cpptui::ToggleSwitch> m_toggle{nullptr};
};

}  // namespace clover2_ui::tui::components
