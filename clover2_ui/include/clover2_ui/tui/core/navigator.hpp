#pragma once

#include <clover2_ui/tui/core/screen.hpp>
#include <clover2_ui/tui/widget/footer.hpp>
#include <cpptui/cpptui.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace clover2_ui::tui::core {

class navigator : public cpptui::Vertical {
public:
    navigator(cpptui::App& app, const std::string& title = "");

    void push(std::shared_ptr<screen> screen);
    void pop();
    void replace(std::shared_ptr<screen> screen);
    bool can_go_back() const;
    size_t depth() const { return m_stack.size(); }

    std::shared_ptr<screen> current() const;

    struct key_binding {
        int key;
        bool ctrl = false;
    };

    void add_key_binding(key_binding binding,
                         std::function<void()> callback,
                         std::string key = "",
                         std::string description = "");

    bool on_event(const cpptui::Event& event) override;

private:
    void update_display();
    void update_header();
    void update_footer();

    cpptui::App& m_app;
    std::vector<std::shared_ptr<screen>> m_stack;

    struct bound_key {
        key_binding binding;
        std::function<void()> callback;
        std::string key;
        std::string description;
    };

    std::shared_ptr<cpptui::Border> m_header;
    std::shared_ptr<cpptui::Label> m_header_label;
    std::shared_ptr<cpptui::Vertical> m_content;
    std::shared_ptr<widget::footer> m_footer;

    std::vector<bound_key> m_key_bindings;
};

}  // namespace clover2_ui::tui::core
