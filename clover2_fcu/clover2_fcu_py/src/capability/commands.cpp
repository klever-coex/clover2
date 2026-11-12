#include <clover2_fcu_py/capability/commands.hpp>

// clover2
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/data/flight_mode.hpp>

// STL
#include <optional>
#include <string>
#include <utility>

namespace clover2_fcu_py::capability {

namespace {

using clover2_fcu::capability::commands_interface;

/// Wraps a Python callback into the ack-bool result_callback: safe to run on
/// client spin threads, a Python exception inside cannot kill the C++ thread.
commands_interface::result_callback make_callback(pybind11::function cb) {
    return [cb = std::move(cb)](bool ok) {
        pybind11::gil_scoped_acquire acquire;
        try {
            cb(ok);
        } catch (pybind11::error_already_set& e) {
            e.discard_as_unraisable("clover2_fcu commands callback");
        }
    };
}

std::optional<double> opt_double(const pybind11::object& value) {
    if (value.is_none()) {
        return std::nullopt;
    }

    return value.cast<double>();
}

}  // namespace

void bind_commands(pybind11::object module) {
    pybind11::class_<commands_interface, commands_interface::shared_ptr>(
        module, "Commands")
        .def(
            "arm",
            [](commands_interface& self, pybind11::function cb,
               bool run_preflight_checks) {
                self.arm(make_callback(std::move(cb)), run_preflight_checks);
            },
            pybind11::arg("cb"), pybind11::arg("run_preflight_checks") = false,
            "Arm the vehicle; cb(ok) fires when the FCU acknowledges.")
        .def(
            "disarm",
            [](commands_interface& self, pybind11::function cb) {
                self.disarm(make_callback(std::move(cb)));
            },
            pybind11::arg("cb"),
            "Disarm the vehicle; cb(ok) fires when the FCU acknowledges.")
        .def(
            "set_mode",
            [](commands_interface& self, pybind11::function cb,
               const std::string& mode) {
                self.set_mode(make_callback(std::move(cb)),
                              clover2_fcu::data::flight_mode(mode));
            },
            pybind11::arg("cb"), pybind11::arg("mode"),
            "Switch the flight mode by name; cb(ok) fires on the ack.")
        .def(
            "takeoff",
            [](commands_interface& self, pybind11::function cb,
               const pybind11::object& altitude,
               const pybind11::object& heading) {
                self.takeoff(make_callback(std::move(cb)), opt_double(altitude),
                             opt_double(heading));
            },
            pybind11::arg("cb"), pybind11::arg("altitude") = pybind11::none(),
            pybind11::arg("heading") = pybind11::none(),
            "Trigger an automatic takeoff; cb(ok) is the FCU ack, not the "
            "end of the maneuver.")
        .def(
            "land",
            [](commands_interface& self, pybind11::function cb) {
                self.land(make_callback(std::move(cb)));
            },
            pybind11::arg("cb"),
            "Trigger an automatic landing; cb(ok) is the FCU ack, not the "
            "touchdown.")
        .def(
            "rtl",
            [](commands_interface& self, pybind11::function cb) {
                self.rtl(make_callback(std::move(cb)));
            },
            pybind11::arg("cb"),
            "Trigger return-to-launch; cb(ok) is the FCU ack, not the "
            "landing.");
}

}  // namespace clover2_fcu_py::capability
