#pragma once

#include <clover2_ui/tui/core/screen.hpp>
#include <cpptui/cpptui.hpp>

#include <memory>

namespace clover2_ui::tui::core {

class centered : public screen {
public:
    centered(std::shared_ptr<screen> inner, int width = 0, int height = 0);

    void on_enter() override;
    void on_exit() override;
    void set_navigator(std::weak_ptr<core::navigator> nav) override;
    std::vector<std::pair<std::string, std::string>> shortcuts() const override;

    std::shared_ptr<screen> inner() const { return m_inner; }

private:
    std::shared_ptr<cpptui::Align> m_align;
    std::shared_ptr<screen> m_inner;
};

}  // namespace clover2_ui::tui::core
