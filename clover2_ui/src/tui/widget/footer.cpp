#include <clover2_ui/tui/widget/footer.hpp>
#include <cpptui/cpptui.hpp>

#include <string>
#include <utility>
#include <vector>

namespace clover2_ui::tui::widget {

footer::footer()
    : cpptui::Border(cpptui::BorderStyle::ASCII) {
    fixed_height = 3;

    m_bar = std::make_shared<cpptui::ShortcutBar>();
    add(m_bar);
}

void footer::set_shortcuts(
    std::vector<std::pair<std::string, std::string>> shortcuts) {
    m_bar->items.clear();
    for (auto& [key, desc] : shortcuts) {
        m_bar->add(key, cpptui::StyledText(desc));
    }
}

void footer::add_shortcut(std::pair<std::string, std::string> shortcut) {
    m_bar->add(shortcut.first, cpptui::StyledText(shortcut.second));
}

}  // namespace clover2_ui::tui::widget
