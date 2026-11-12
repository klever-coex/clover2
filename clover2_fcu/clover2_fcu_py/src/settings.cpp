#include <clover2_fcu_py/settings.hpp>

// pybind
#include <pybind11/stl.h>

namespace clover2_fcu_py {

bool settings::get_use_diagnostics() const {
    return m_settings.use_diagnostics();
}

void settings::set_use_diagnostics(bool val) {
    m_settings.use_diagnostics(val);
}

const std::string& settings::get_bridge_node() const {
    return m_settings.bridge_node();
}

void settings::set_bridge_node(const std::string& val) {
    m_settings.bridge_node(val);
}

double settings::get_wait_timeout() const {
    return std::chrono::duration<double>(m_settings.wait_timeout()).count();
}

void settings::set_wait_timeout(double seconds) {
    m_settings.wait_timeout(std::chrono::duration<double>(seconds));
}

std::vector<std::string> settings::get_required() const {
    return m_settings.required();
}

void settings::require(const std::string& name) { m_settings.require(name); }

void bind_settings(pybind11::object module) {
    pybind11::class_<settings, std::shared_ptr<settings>>(module, "Settings")
        .def(pybind11::init<>())
        .def_property("use_diagnostics", &settings::get_use_diagnostics,
                      &settings::set_use_diagnostics)
        .def_property("bridge_node", &settings::get_bridge_node,
                      &settings::set_bridge_node)
        .def_property("wait_timeout", &settings::get_wait_timeout,
                      &settings::set_wait_timeout)
        .def_property_readonly("required", &settings::get_required)
        .def("require", &settings::require, pybind11::arg("capability"),
             "Require a capability: constructing the client fails when its "
             "backend does not serve it.");
}

}  // namespace clover2_fcu_py
