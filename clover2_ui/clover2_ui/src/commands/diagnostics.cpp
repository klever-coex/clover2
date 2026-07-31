#include <clover2_ui/api/diagnostics/diagnostic_monitor.hpp>
#include <clover2_ui/tui/components/diagnostics_screen.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <cpptui/cpptui.hpp>
#include <rclcpp/rclcpp.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using namespace clover2_ui;
using namespace std::chrono_literals;

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
    auto monitor =
        std::make_shared<api::diagnostics::diagnostic_monitor>(opts.topic);
    auto nav =
        std::make_shared<tui::core::navigator>(app, "Clover2 diagnostics");
    auto screen =
        std::make_shared<tui::components::diagnostics_screen>(monitor, nav);

    std::atomic_bool running = true;

    std::thread ros_thread([&]() { rclcpp::spin(monitor->node()); });

    std::thread refresh_thread([&]() {
        while (running && rclcpp::ok()) {
            app.post([screen]() { screen->refresh_from_monitor(); });
            std::this_thread::sleep_for(500ms);
        }
    });

    nav->push(screen);
    app.run(nav);

    running = false;
    rclcpp::shutdown();

    if (refresh_thread.joinable()) refresh_thread.join();
    if (ros_thread.joinable()) ros_thread.join();

    return 0;
}
