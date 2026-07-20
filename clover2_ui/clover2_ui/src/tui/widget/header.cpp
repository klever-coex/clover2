#include <clover2_ui/tui/widget/header.hpp>
#include <cpptui/cpptui.hpp>

#include <string>

namespace clover2_ui::tui::widget {

header::header(const std::string& app_name)
    : cpptui::Border(cpptui::BorderStyle::ASCII)
    , m_app_name(app_name) {
    fixed_height = 3;

    auto app_name_label = std::make_shared<cpptui::Label>(
        m_app_name, cpptui::Theme::current().primary);

    set_title(m_app_name, cpptui::Alignment::Center);
    add(app_name_label);
}

}  // namespace clover2_ui::tui::widget
