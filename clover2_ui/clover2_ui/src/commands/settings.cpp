#include <clover2_ui/api/settings/config_field.hpp>
#include <clover2_ui/tui/components/field_editor.hpp>
#include <clover2_ui/tui/components/form_screen.hpp>
#include <clover2_ui/tui/core/centered.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <clover2_ui/tui/core/screen.hpp>
#include <cpptui/cpptui.hpp>

#include <filesystem>
#include <memory>
#include <string>

using namespace clover2_ui;

static std::shared_ptr<tui::core::screen> create_form_screen(
    std::shared_ptr<api::settings::config_field>& field,
    std::shared_ptr<tui::core::navigator>& nav);

static void navigate_to_field(
    std::shared_ptr<api::settings::config_field> field,
    std::shared_ptr<tui::core::navigator> nav) {
    if (field->is_object()) {
        nav->push(create_form_screen(field, nav));
    } else {
        auto editor =
            std::make_shared<tui::components::field_editor>(field, nav);
        nav->push(std::make_shared<tui::core::centered>(editor, 50, 10));
    }
}

static std::shared_ptr<tui::core::screen> create_form_screen(
    std::shared_ptr<api::settings::config_field>& field,
    std::shared_ptr<tui::core::navigator>& nav) {
    auto form = std::make_shared<tui::components::form_screen>(
        field,
        [nav](std::shared_ptr<api::settings::config_field> child) {
            navigate_to_field(child, nav);
        },
        nav);

    auto centered = std::make_shared<tui::core::centered>(form, 50, 10);

    return centered;
}

int main(int argc, char** argv) {
    cpptui::App app;

    if (argc < 3) {
        std::cout << "A command line utility for edit clover2 settings via TUI."
                  << std::endl;
        std::cout << "Usage: settings <path to yaml schema> <output yaml>"
                  << std::endl;
        std::cout << "Example: settings $(ros2 pkg prefix "
                     "clover2_ui)/schemas/klever5.yaml ~/config.yaml"
                  << std::endl;
        return 0;
    }

    const std::filesystem::path schema_path = argv[1];
    const std::filesystem::path values_path = argv[2];

    auto root_field =
        api::settings::config_field::load(schema_path, values_path);
    auto nav = std::make_shared<tui::core::navigator>(app, "Clover2 settings");

    nav->add_key_binding(
        {'s', true},
        [&]() {
            try {
                root_field->save_to_file(values_path);
                root_field->mark_clean();

                nav->show_notification("File saved.",
                                       cpptui::Notification::Type::Success,
                                       1000);
            } catch (const std::exception& e) {
                nav->show_notification(e.what(),
                                       cpptui::Notification::Type::Error, 1000);
            }
        },
        "ctrl^s", "Save");

    auto root_form = create_form_screen(root_field, nav);

    nav->push(root_form);
    app.run(nav);

    return 0;
}
