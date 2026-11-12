#include <clover2_fcu_py/capability/system_state.hpp>

// clover2
#include <clover2_fcu/capability/system_state_interface.hpp>

namespace clover2_fcu_py::capability {

namespace {

pybind11::dict state_to_dict(const clover2_fcu::data::system_state& s) {
    pybind11::dict out;
    out["connected"] = s.connected;
    out["armed"] = s.armed;
    out["in_air"] = s.in_air;
    out["failsafe"] = s.failsafe;
    out["mode"] = std::string(s.mode.name());
    return out;
}

using state_token = clover2_fcu::capability::system_state_interface::token;

}  // namespace

void bind_system_state(pybind11::object module) {
    pybind11::class_<state_token>(module, "Subscription")
        .def("close", &state_token::release,
             "Unsubscribe; a closed subscription stays closed.");

    pybind11::class_<
        clover2_fcu::capability::system_state_interface,
        std::shared_ptr<clover2_fcu::capability::system_state_interface>>(
        module, "SystemState")
        .def("has_data",
             &clover2_fcu::capability::system_state_interface::has_data,
             "Whether at least one snapshot was received.")
        .def(
            "last",
            [](const clover2_fcu::capability::system_state_interface& self) {
                return state_to_dict(self.last());
            },
            "Latest snapshot as a dict; raises RuntimeError until data "
            "arrives.")
        .def(
            "last_valid",
            [](const clover2_fcu::capability::system_state_interface& self,
               double max_delay) {
                return self.last_valid(
                    std::chrono::duration<double>(max_delay));
            },
            pybind11::arg("max_delay") = 0.5,
            "Whether the latest snapshot is fresher than max_delay seconds.")
        .def(
            "on_update",
            [](clover2_fcu::capability::system_state_interface& self,
               pybind11::function callback) {
                return self.on_update(
                    [callback](const clover2_fcu::data::system_state& s) {
                        pybind11::gil_scoped_acquire acquire;
                        callback(state_to_dict(s));
                    });
            },
            pybind11::arg("callback"),
            "Subscribe a callback(state_dict); returns a Subscription.");
}

}  // namespace clover2_fcu_py::capability
