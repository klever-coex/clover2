#include <clover2_common/diagnostics/client.hpp>
#include <clover2_common/node_runtime.hpp>
#include <clover2_ui/api/diagnostics/diagnostic_model.hpp>
#include <clover2_ui/tui/components/diagnostics_screen.hpp>
#include <clover2_ui/tui/core/navigator.hpp>
#include <cpptui/cpptui.hpp>
#include <rclcpp/rclcpp.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

using namespace clover2_ui;

namespace {

struct options {
    std::string topic = clover2_common::diagnostics::client::default_topic;
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
    std::cout << "Usage: diagnostics [--topic "
              << clover2_common::diagnostics::client::default_topic << "]"
              << std::endl;
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
    auto node_context = runtime.get_node_context();

    auto diagnostics_client =
        std::make_shared<clover2_common::diagnostics::client>(node_context,
                                                             opts.topic);
    api::diagnostics::model diagnostics_model;
    std::mutex diagnostics_mutex;
    rclcpp::Time last_update;

    auto nav =
        std::make_shared<tui::core::navigator>(app, "Clover2 diagnostics");
    auto screen = std::make_shared<tui::components::diagnostics_screen>(
        [&diagnostics_mutex, &diagnostics_model, &last_update,
         node_context]() {
            std::lock_guard<std::mutex> lock(diagnostics_mutex);
            auto out = diagnostics_model.get_snapshot();
            if (out.received) {
                const auto now =
                    node_context->get_node_clock_interface()->get_clock()->now();
                out.age_sec = (now - last_update).seconds();
            }
            return out;
        },
        opts.topic, nav);

    diagnostics_client->set_callback(
        [&diagnostics_mutex, &diagnostics_model, &last_update, node_context](
            const clover2_common::diagnostics::client::message_type& msg) {
            std::lock_guard<std::mutex> lock(diagnostics_mutex);
            diagnostics_model.update(msg);
            last_update =
                node_context->get_node_clock_interface()->get_clock()->now();
        });

    runtime.start();
    const auto refresh_timer =
        app.add_timer(500, [screen]() { screen->refresh(); });

    nav->push(screen);
    app.run(nav);

    app.remove_timer(refresh_timer);
    diagnostics_client->cleanup();
    runtime.stop();
    rclcpp::shutdown();

    return 0;
}
