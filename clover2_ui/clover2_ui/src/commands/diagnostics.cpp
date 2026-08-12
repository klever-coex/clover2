#include <clover2_common/node_runtime.hpp>
#include <clover2_ui/api/diagnostics/diagnostic_monitor.hpp>
#include <clover2_ui/tui/components/diagnostics_screen.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <cpptui/cpptui.hpp>
#include <rclcpp/rclcpp.hpp>

#include <iostream>
#include <memory>
#include <string>

using namespace clover2_ui;

namespace {

struct options {
    std::string topic = "/diagnostics_agg";
    bool help = false;
};

options parse_options(int argc, char** argv) {
    options out;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            out.help = true;
        } else if ((arg == "--topic" || arg == "-t") && i + 1 < argc) {
            out.topic = argv[++i];
        }
    }

    return out;
}

void print_usage() {
    std::cout << "A terminal UI monitor for Clover2 diagnostics." << std::endl;
    std::cout << "Usage: diagnostics [--topic /diagnostics_agg]" << std::endl;
    std::cout << "Example: ros2 run clover2_ui diagnostics" << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    auto opts = parse_options(argc, argv);
    if (opts.help) {
        print_usage();
        return 0;
    }

    rclcpp::init(argc, argv);

    cpptui::App app;
    clover2_common::node_runtime runtime("clover2_diagnostics_tui");
    auto monitor = std::make_shared<api::diagnostics::monitor>(
        runtime.get_node_context(), opts.topic);
    auto nav =
        std::make_shared<tui::core::navigator>(app, "Clover2 diagnostics");
    auto screen = std::make_shared<tui::components::diagnostics_screen>(
        monitor, nav);

    runtime.start();
    const auto refresh_timer =
        app.add_timer(500, [screen]() { screen->refresh_from_monitor(); });

    nav->push(screen);
    app.run(nav);

    app.remove_timer(refresh_timer);
    runtime.stop();
    rclcpp::shutdown();

    return 0;
}
