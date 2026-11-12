#pragma once

// pybind
#include <pybind11/pybind11.h>

namespace clover2_fcu_py::capability {

void bind_commands(pybind11::object module);

}  // namespace clover2_fcu_py::capability
