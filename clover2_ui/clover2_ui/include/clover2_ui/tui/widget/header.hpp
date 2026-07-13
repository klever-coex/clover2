#pragma once

#include <cpptui/cpptui.hpp>

#include <string>

namespace clover2_ui::tui::widget {

class header : public cpptui::Border {
public:
    header(const std::string& app_name);

private:
    std::string m_app_name;
};

}  // namespace clover2_ui::tui::widget
