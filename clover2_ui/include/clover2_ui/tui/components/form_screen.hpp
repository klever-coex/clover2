#pragma once

#include <clover2_ui/api/settings/config_field.hpp>
#include <clover2_ui/tui/core/screen.hpp>
#include <cpptui/cpptui.hpp>

#include <functional>
#include <memory>

namespace clover2_ui::tui::components {

using navigate_fn =
    std::function<void(std::shared_ptr<api::settings::config_field> field)>;

class form_screen : public core::screen {
public:
    form_screen(std::shared_ptr<api::settings::config_field> object,
               navigate_fn on_navigate,
               std::shared_ptr<core::navigator> nav = nullptr);

    bool on_event(const cpptui::Event& event) override;
    void on_focus() override;
    void on_enter() override;

    std::vector<std::pair<std::string, std::string>> shortcuts() const override;

private:
    size_t m_current{0};

    navigate_fn m_on_navigate;

    std::shared_ptr<api::settings::config_field> m_object;
    std::shared_ptr<cpptui::Label> m_title_label;
    std::vector<std::shared_ptr<cpptui::Button>> m_buttons;
};

}  // namespace clover2_ui::tui::components
