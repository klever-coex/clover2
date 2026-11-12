#include <clover2_fcu_py/capability/all.hpp>
#include <clover2_fcu_py/client.hpp>

// clover2
#include <clover2_fcu/capability/all.hpp>

// pybind
#include <pybind11/stl.h>

// STL
#include <cstdio>
#include <random>

namespace clover2_fcu_py {

using binder_fn =
    std::function<pybind11::object(clover2_fcu::client::shared_ptr)>;

namespace {

std::atomic<int> g_instances{0};
bool g_we_initialized = false;

std::string random_node_name() {
    static std::mt19937_64 rng{std::random_device{}()};
    char suffix[9];
    snprintf(suffix, sizeof(suffix), "%08x",
             static_cast<unsigned int>(rng() & 0xFFFFFFFFU));

    return "fcu_client_" + std::string(suffix);
}

void ensure_rclcpp_init() {
    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
        g_we_initialized = true;
    }

    ++g_instances;
}

void release_rclcpp() {
    if (--g_instances == 0 && g_we_initialized) {
        rclcpp::shutdown();
    }
}

}  // namespace

client::client()
    : client(settings{}) {}

client::client(const settings& s)
    : m_settings(s) {
    ensure_rclcpp_init();

    try {
        m_runtime =
            std::make_shared<clover2_common::node_runtime>(random_node_name());
        m_runtime->start();

        m_client = std::make_shared<clover2_fcu::client>(*m_runtime->get_node(),
                                                         m_settings.raw());
    } catch (const std::exception&) {
        if (m_runtime) {
            m_runtime->stop();
            m_runtime.reset();
        }

        release_rclcpp();
        throw;
    }
}

client::~client() {
    m_client.reset();
    m_runtime->stop();
    m_runtime.reset();
    release_rclcpp();
}

std::string client::node_name() const {
    return m_runtime->get_node()->get_name();
}

bool client::has(const std::string& capability) {
    return m_client->has(capability);
}

template <typename T>
std::pair<std::string, binder_fn> capability_builder() {
    return {T::name, [](clover2_fcu::client::shared_ptr c) -> pybind11::object {
                return pybind11::cast(c->get<T>());
            }};
}

pybind11::object client::getitem(const std::string& capability) {
    static const std::unordered_map<std::string, binder_fn> k_binders = {
        capability_builder<clover2_fcu::capability::system_state_interface>(),
        capability_builder<clover2_fcu::capability::commands_interface>(),
    };

    const auto it = k_binders.find(capability);
    if (it != k_binders.end()) {
        return it->second(m_client);
    }

    for (const auto& known : clover2_fcu::capability::all_names()) {
        if (known == capability) {
            throw std::runtime_error("capability '" + capability +
                                     "' is served by the backend but is not "
                                     "exposed in Python yet");
        }
    }

    throw pybind11::key_error("unknown capability '" + capability + "';");
}

void bind_client(pybind11::object module) {
    capability::bind_all(module);

    pybind11::class_<client, std::shared_ptr<client>>(module, "FCUClient")
        .def(pybind11::init<>(),
             "Connect to the bridge; the private node is spun in the "
             "background.")
        .def(pybind11::init<const settings&>(), pybind11::arg("settings"))
        .def_property_readonly("node_name", &client::node_name,
                               "Random name of the private node.")
        .def("has", &client::has, pybind11::arg("capability"),
             "Whether the backend serves the capability.")
        .def("__getitem__", &client::getitem, pybind11::arg("capability"),
             "Capability provider, cached: fcu[\"system_state\"].");

    module.attr("Client") = module.attr("FCUClient");
}

}  // namespace clover2_fcu_py
