#include <clover2_ui/tui/core/navigator.hpp>
#include <cpptui/cpptui.hpp>

#include <cctype>
#include <memory>
#include <string>

namespace clover2_ui::tui::core {

navigator::navigator(cpptui::App& app, const std::string& title)
    : cpptui::Vertical()
    , m_app(app) {
    m_header = std::make_shared<cpptui::Border>(cpptui::BorderStyle::Single);
    m_header->set_title(title, cpptui::Alignment::Center);
    m_header->fixed_height = 3;
    add(m_header);

    m_header_label = std::make_shared<cpptui::Label>(
        title.empty() ? "" : title, cpptui::Theme::current().primary);
    m_header->add(m_header_label);

    m_content = std::make_shared<cpptui::Vertical>();
    add(m_content);

    m_footer = std::make_shared<widget::footer>();
    add(m_footer);

    m_app.register_exit_key('q', true);
}

void navigator::push(std::shared_ptr<screen> screen) {
    if (!screen) return;

    screen->set_navigator(
        std::static_pointer_cast<navigator>(shared_from_this()));

    if (!m_stack.empty()) {
        m_stack.back()->on_exit();
    }

    m_stack.push_back(std::move(screen));
    m_stack.back()->on_enter();

    update_display();
}

void navigator::pop() {
    if (m_stack.size() <= 1) return;

    m_stack.back()->on_exit();
    m_stack.pop_back();
    m_stack.back()->on_enter();

    update_display();
}

void navigator::replace(std::shared_ptr<screen> screen) {
    if (!screen) return;

    screen->set_navigator(
        std::static_pointer_cast<navigator>(shared_from_this()));

    if (!m_stack.empty()) {
        m_stack.back()->on_exit();
        m_stack.pop_back();
    }

    m_stack.push_back(std::move(screen));
    m_stack.back()->on_enter();

    update_display();
}

void navigator::add_key_binding(key_binding binding,
                                std::function<void()> callback,
                                std::string key,
                                std::string description) {
    m_key_bindings.push_back(
        {std::move(binding), std::move(callback), std::move(key),
         std::move(description)});
}

bool navigator::can_go_back() const { return m_stack.size() > 1; }

std::shared_ptr<screen> navigator::current() const {
    if (m_stack.empty()) return nullptr;
    return m_stack.back();
}

bool navigator::on_event(const cpptui::Event& event) {
    if (event.is_key_event()) {
        if (event.is_escape()) {
            if (can_go_back()) {
                m_app.post([this]() { pop(); });
                return true;
            }
            return true;
        }

        if (event.is_backspace()) {
            if (can_go_back()) {
                m_app.post([this]() { pop(); });
                return true;
            }
            return true;
        }

        for (const auto& bk : m_key_bindings) {
            bool match = false;
            if (bk.binding.ctrl) {
                char upper = std::toupper(
                    static_cast<char>(bk.binding.key));
                char lower = std::tolower(
                    static_cast<char>(bk.binding.key));
                int raw = upper - 'A' + 1;
                match = (event.ctrl &&
                         (event.key == upper || event.key == lower ||
                          event.key == raw));
            } else {
                match = (event.key == bk.binding.key &&
                         !event.ctrl);
            }
            if (match) {
                m_app.post([cb = bk.callback]() { cb(); });
                return true;
            }
        }
    }

    return cpptui::Vertical::on_event(event);
}

void navigator::update_display() {
    m_content->clear_children();
    if (!m_stack.empty()) {
        m_content->add(m_stack.back());
        m_content->layout();
    }
    update_header();
    update_footer();
}

void navigator::update_footer() {
    std::vector<std::pair<std::string, std::string>> all = {
        {"ctrl^q", "Quit"},
        {"backspace", "Back"},
    };

    for (const auto& bk : m_key_bindings) {
        if (!bk.key.empty()) {
            all.push_back({bk.key, bk.description});
        }
    }

    if (!m_stack.empty()) {
        auto extra = m_stack.back()->shortcuts();
        all.insert(all.end(), extra.begin(), extra.end());
    }

    m_footer->set_shortcuts(all);
}

void navigator::update_header() {
    if (m_stack.empty()) return;
    std::string title = m_stack.back()->title();
    m_header_label->set_text(title);
}

}  // namespace clover2_ui::tui::core
