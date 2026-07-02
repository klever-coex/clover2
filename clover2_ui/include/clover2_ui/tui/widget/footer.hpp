#pragma once

#include <cpptui/cpptui.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace clover2_ui::tui::widget {

class footer : public cpptui::Border {
public:
    footer();

    void set_shortcuts(
        std::vector<std::pair<std::string, std::string>> shortcuts);
    void add_shortcut(std::pair<std::string, std::string> shortcut);

private:
    std::shared_ptr<cpptui::ShortcutBar> m_bar;
};

}  // namespace clover2_ui::tui::widget
