#include <clover2_ui/tui/core/centered.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <cpptui/cpptui.hpp>

#include <memory>

namespace clover2_ui::tui::core {

centered::centered(std::shared_ptr<screen> inner, int width, int height)
    : screen(inner->name())
    , m_inner(std::move(inner)) {
    m_title = m_inner->title();

    m_align = std::make_shared<cpptui::Align>(cpptui::Align::H::Center,
                                              cpptui::Align::V::Center);

    if (width > 0) m_inner->fixed_width = width;
    if (height > 0) m_inner->fixed_height = height;

    m_align->add(m_inner);
    add(m_align);
}

void centered::on_enter() { m_inner->on_enter(); }

void centered::on_exit() { m_inner->on_exit(); }

void centered::set_navigator(std::shared_ptr<core::navigator> nav) {
    screen::set_navigator(nav);
    m_inner->set_navigator(std::move(nav));
}

std::vector<std::pair<std::string, std::string>> centered::shortcuts() const {
    return m_inner->shortcuts();
}

}  // namespace clover2_ui::tui::core
