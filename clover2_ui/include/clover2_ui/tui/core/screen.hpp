#pragma once

#include <cpptui/cpptui.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace clover2_ui::tui::core {

class navigator;

class screen : public cpptui::Vertical {
public:
    screen(std::string name,
           std::shared_ptr<core::navigator> navigator = nullptr)
        : cpptui::Vertical()
        , m_name(std::move(name))
        , m_navigator(std::move(navigator)) {}

    const std::string& name() const { return m_name; }

    const std::string& title() const {
        return m_title.empty() ? m_name : m_title;
    }
    void set_title(const std::string& t) { m_title = t; }

    bool can_go_back() const { return m_can_go_back; }
    void set_can_go_back(bool v) { m_can_go_back = v; }

    std::shared_ptr<core::navigator> get_navigator() const {
        return m_navigator.lock();
    }

    virtual void set_navigator(std::weak_ptr<core::navigator> nav) {
        m_navigator = nav;
    }

    virtual std::vector<std::pair<std::string, std::string>> shortcuts() const {
        return {};
    }

    virtual void on_enter() {}
    virtual void on_exit() {}

protected:
    std::string m_name;
    std::string m_title;
    std::weak_ptr<core::navigator> m_navigator;

private:
    bool m_can_go_back = true;
};

}  // namespace clover2_ui::tui::core
